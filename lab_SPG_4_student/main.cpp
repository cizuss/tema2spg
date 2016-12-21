//-------------------------------------------------------------------------------------------------
// Descriere: fisier main
//
// Autor: student
// Data: today
//-------------------------------------------------------------------------------------------------

#include "lab_mesh_loader.hpp"
#include "lab_geometry.hpp"
#include "lab_shader_loader.hpp"
#include "lab_glut.hpp"
#include "lab_texture_loader.hpp"
#include "lab_camera.hpp"
#include <ctime>
#define PI 3.14


class Laborator : public lab::glut::WindowListener{

//variabile
private:
    lab::Camera camera;
	glm::vec3 up;
	glm::vec3 forward;
	glm::vec3 cameraPosition;
	glm::mat4 projection_matrix;	//matrici 4x4 pt modelare vizualizare proiectie
	glm::mat4 new_view_matrix;
	glm::mat4 skydome_model_matrix;
	unsigned int shader_cubemap, shader_normal;
	unsigned int direction;
	float angle;
	//id-ul de opengl al obiectului de tip program shader

	//meshe
	lab::Mesh mesh_cubemap;	
    lab::Mesh mesh_bunny;
	lab::SolidSphere sphere;
	
	//texturi
	unsigned int texture_cubemap, texture_sky, texture_ground;

	lab::Mesh skydome, ground;



//metode
public:
	
	//constructor .. e apelat cand e instantiata clasa
	Laborator(){
		glClearColor(0.5,0.5,0.5,1);
		glClearDepth(1);
		glEnable(GL_DEPTH_TEST);
		angle = 0.0f;
		direction = 1;
		skydome_model_matrix = glm::mat4(1.0f);
		
        shader_normal = lab::loadShader("shadere/normal_vertex.glsl", "shadere/normal_fragment.glsl");
        shader_cubemap = lab::loadShader("shadere/cubemap_vertex.glsl", "shadere/cubemap_fragment.glsl");

		generateSkyDome(600, 250, 500, skydome);
		generateGround(600, ground);
		lab::loadObj("resurse\\box.obj",mesh_cubemap);	
        lab::loadObj("resurse\\bunny.obj", mesh_bunny);
       //texture_cubemap = lab::loadTextureCubemapBMP("resurse/posx.bmp", "resurse/posy.bmp", "resurse/posz.bmp", "resurse/negx.bmp", "resurse/negy.bmp", "resurse/negz.bmp");
	   texture_ground = lab::loadTextureBMP("resurse/ground.bmp");
	   texture_sky = lab::loadTextureBMP("resurse/sky6.bmp");
        //matrici de modelare si vizualizare
		camera.set(glm::vec3(0,0,40), glm::vec3(0,0,0), glm::vec3(0,1,0));
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		//sphere = lab::SolidSphere(1, 14, 26);
		up = glm::vec3(0, 1, 0);
		forward = camera.getForward();
	}

	//destructor .. e apelat cand e distrusa clasa
	~Laborator(){
		//distruge shadere
		glDeleteProgram(shader_cubemap);
        glDeleteProgram(shader_normal);

		//distruge obiecte
		//glDeleteTextures(1, &texture_cubemap);
		glDeleteTextures(1, &texture_sky);
		glDeleteTextures(1, &texture_ground);
	}
		
	void displaySphere()
	{
		int const win_width = 600; // retrieve window dimensions from
		int const win_height = 800; // framework of choice here
		float const win_aspect = (float)win_width / (float)win_height;

		glViewport(0, 0, win_width, win_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(45, win_aspect, 1, 10);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

#ifdef DRAW_WIREFRAME
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
#endif
		sphere.draw(0, 0, -5);

	}

	glm::vec3 par(float r, float u, float v) {
		return glm::vec3(cos(u) * sin(v) * r, cos(v) * r, sin(u) * sin(v) * r);
	}

	glm::vec2 get_texcoords(float theta, float phi) {
		return glm::vec2(float(theta / (2 * PI)), float( phi / PI));
	}

	GLvoid draw_circle(const GLfloat radius, const GLuint num_vertex, unsigned int &texID)
	{
		GLfloat vertex[3];
		GLfloat texcoord[2];

		const GLfloat delta_angle = 2.0*PI / num_vertex;

		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, texID);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glBegin(GL_TRIANGLE_FAN);

		//draw the vertex at the center of the circle
		texcoord[0] = 0.5;
		texcoord[1] = 0.5;
		glTexCoord2fv(texcoord);

		vertex[0] = vertex[1] = vertex[2] = 0.0;
		//vertex[3] = 1.0;
		glVertex3fv(vertex);

		for (int i = 0; i < num_vertex; i++)
		{
			texcoord[0] = (std::cos(delta_angle*i) + 1.0)*0.5;
			texcoord[1] = (std::sin(delta_angle*i) + 1.0)*0.5;
			glTexCoord2fv(texcoord);

			vertex[0] = std::cos(delta_angle*i) * radius;
			vertex[2] = std::sin(delta_angle*i) * radius;
			vertex[1] = 0.0;
			//vertex[3] = 1.0;
			glVertex3fv(vertex);
		}

		texcoord[0] = (1.0 + 1.0)*0.5;
		texcoord[1] = (0.0 + 1.0)*0.5;
		glTexCoord2fv(texcoord);

		vertex[0] = 1.0 * radius;
		vertex[2] = 0.0 * radius;
		vertex[1] = 0.0;
		//vertex[3] = 1.0;
		glVertex3fv(vertex);
		glEnd();

		glDisable(GL_TEXTURE_2D);

	}

	void generateGround(float radius, lab::Mesh &mesh) {
		//lab::Mesh mesh = lab::Mesh();
		std::vector<lab::VertexFormat> vertices;
		std::vector<unsigned int> indices;

		glm::vec3 p;
		glm::vec3 norm;
		glm::vec2 tex;

		float u, v;
		float x, y, z;

		int num_vertex = 200;

		p = glm::vec3(0, 0, 0);
		norm = glm::vec3(0, 0, 0);
		tex = glm::vec2(0.5f, 0.5f);
		int k = 0;
		vertices.push_back(lab::VertexFormat(p, norm, tex));
		float deltaAngle = 2 * PI / num_vertex;
		for (int i = -3; i < num_vertex; i++) {
			u = (cos(i * deltaAngle) + 1.0f) * 0.5f;
			v = (sin(i * deltaAngle) + 1.0f) * 0.5f;
			tex = glm::vec2(u, v);
			x = cos(i * deltaAngle) * radius;
			y = 0.0f;
			z = sin(i * deltaAngle) * radius;
			p = glm::vec3(x, y, z);
			norm = glm::normalize(p);
			vertices.push_back(lab::VertexFormat(p, norm, tex));
			//indices.push_back(0);
			indices.push_back(k++);
		}

		indices.push_back(k++);

		unsigned int gl_vertex_array_object, gl_vertex_buffer_object, gl_index_buffer_object;
		glGenVertexArrays(1, &gl_vertex_array_object);
		glBindVertexArray(gl_vertex_array_object);

		//vertex buffer object -> un obiect in care tinem vertecsii
		glGenBuffers(1, &gl_vertex_buffer_object);
		glBindBuffer(GL_ARRAY_BUFFER, gl_vertex_buffer_object);
		glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(lab::VertexFormat), &vertices[0], GL_STATIC_DRAW);

		//index buffer object -> un obiect in care tinem indecsii
		glGenBuffers(1, &gl_index_buffer_object);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_index_buffer_object);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		//legatura intre atributele vertecsilor si pipeline, datele noastre sunt INTERLEAVED.
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(lab::VertexFormat), (void*)0);						//trimite pozitii pe pipe 0
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(lab::VertexFormat), (void*)(sizeof(float) * 3));		//trimite normale pe pipe 1
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(lab::VertexFormat), (void*)(2 * sizeof(float) * 3));	//trimite texcoords pe pipe 2

		mesh.vao = gl_vertex_array_object;
		mesh.vbo = gl_vertex_buffer_object;
		mesh.ibo = gl_index_buffer_object;
		mesh.count = indices.size();

		mesh.setIndices(indices);
		mesh.setVertices(vertices);

		//return mesh;
	}


	void generateSkyDome(float radius, int rings, int sectors, lab::Mesh &mesh) {
		int r, s;
		float theta, phi;

		int UResolution = 25;
		int VResolution = 25;
		float stepU = 2 * PI / UResolution;
		float stepV = PI / VResolution;
		float startU = 0.0f, startV = 0.0f, endU = 2 * PI, endV = PI;

		std::vector<lab::VertexFormat> vertices;
		std::vector<unsigned int> indices;

		float u, v, un, vn;
		glm::vec3 p0, p1, p2, p3;
		glm::vec3 n0, n1, n2, n3;
		glm::vec2 tex;
		int i0, i1, i2, i3;
		int k = 0;
		/*for (int i = 0; i < UResolution; i++) {
			for (int j = 0; j < VResolution / 2; j++) {
				u = i * stepU;
				v = j * stepV;
				un = (i + 1 == UResolution) ? endU : ((i + 1)*stepU + startU);
				vn = (j + 1 == VResolution) ? endV : ((j + 1)*stepV + startV);
				p0 = par(radius, u, v);
				n0 = glm::normalize(p0);
				i0 = k++;
				p1 = par(radius, u, vn);
				n1 = glm::normalize(p1);
				i1 = k++;
				p2 = par(radius, un, v);
				n2 = glm::normalize(p2);
				i2 = k++;
				p3 = par(radius, un, vn);
				n3 = glm::normalize(p3);
				i3 = k++;
				tex = glm::vec2(u / (2 * PI), v / PI);
				vertices.push_back(lab::VertexFormat(p0.x, p0.y, p0.z, n0.x, n0.y, n0.z, tex.x, tex.y));
				tex = glm::vec2(u / (2 * PI), vn / PI);
				vertices.push_back(lab::VertexFormat(p1.x, p1.y, p1.z, n1.x, n0.y, n0.z, tex.x, tex.y));
				tex = glm::vec2(un / (2 * PI), v / PI);
				vertices.push_back(lab::VertexFormat(p2.x, p2.y, p2.z, n2.x, n2.y, n2.z, tex.x, tex.y));
				tex = glm::vec2(un / (2 * PI), vn / PI);
				vertices.push_back(lab::VertexFormat(p3.x, p3.y, p3.z, n3.x, n3.y, n3.z, tex.x, tex.y));
				indices.push_back(i0); indices.push_back(i2); indices.push_back(i1);
				indices.push_back(i3); indices.push_back(i1); indices.push_back(i2);
				//indices.push_back(i0);
				//indices.push_back(i1);
				//indices.push_back(i2);
				//indices.push_back(i3);
				
			}
		} */
		k = 0;
		float step = 0.1f;
		for (theta = 0; theta < 2 * PI; theta += step)
			for (phi = 0; phi < PI / 2; phi += step) {
				//float x = radius * cos(theta) * sin(phi);
				//float y = radius * sin(theta) * sin(phi);
				//float z = radius * cos(theta);
				glm::vec3 norm = glm::vec3(0.0f, 0.0f, 0.0f);
				p0 = par(radius, theta, phi);
				p1 = par(radius, theta + step, phi);
				p2 = par(radius, theta + step, phi + step);
				p3 = par(radius, theta, phi + step);
				i0 = k++; i1 = k++; i2 = k++; i3 = k++;
				tex = glm::vec2(0.0f, 0.0f);
				tex = get_texcoords(theta, phi);
				norm = glm::normalize(p0);
				vertices.push_back(lab::VertexFormat(p0.x, p0.y, p0.z, norm.x, norm.y, norm.z, tex.x, tex.y));
				tex = get_texcoords(theta + step, phi);
				norm = glm::normalize(p1);
				vertices.push_back(lab::VertexFormat(p1.x, p1.y, p1.z, norm.x, norm.y, norm.z, tex.x, tex.y));
				tex = get_texcoords(theta + step, phi + step);
				norm = glm::normalize(p2);
				vertices.push_back(lab::VertexFormat(p2.x, p2.y, p2.z, norm.x, norm.y, norm.z, tex.x, tex.y));
				tex = get_texcoords(theta, phi + step);
				norm = glm::normalize(p3);
				vertices.push_back(lab::VertexFormat(p3.x, p3.y, p3.z, norm.x, norm.y, norm.z, tex.x, tex.y));
				indices.push_back(i0); indices.push_back(i1); indices.push_back(i2);
				indices.push_back(i0); indices.push_back(i3); indices.push_back(i2);
				//indices.push_back(i0);
				//indices.push_back(i1);
				//indices.push_back(i2);
				//indices.push_back(i3);
			}

		//lab::Mesh mesh = lab::Mesh();

		unsigned int gl_vertex_array_object, gl_vertex_buffer_object, gl_index_buffer_object;
		glGenVertexArrays(1, &gl_vertex_array_object);
		glBindVertexArray(gl_vertex_array_object);

		//vertex buffer object -> un obiect in care tinem vertecsii
		glGenBuffers(1, &gl_vertex_buffer_object);
		glBindBuffer(GL_ARRAY_BUFFER, gl_vertex_buffer_object);
		glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(lab::VertexFormat), &vertices[0], GL_STATIC_DRAW);

		//index buffer object -> un obiect in care tinem indecsii
		glGenBuffers(1, &gl_index_buffer_object);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_index_buffer_object);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		//legatura intre atributele vertecsilor si pipeline, datele noastre sunt INTERLEAVED.
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(lab::VertexFormat), (void*)0);						//trimite pozitii pe pipe 0
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(lab::VertexFormat), (void*)(sizeof(float) * 3));		//trimite normale pe pipe 1
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(lab::VertexFormat), (void*)(2 * sizeof(float) * 3));	//trimite texcoords pe pipe 2

		mesh.vao = gl_vertex_array_object;
		mesh.vbo = gl_vertex_buffer_object;
		mesh.ibo = gl_index_buffer_object;
		mesh.count = indices.size();

		mesh.setIndices(indices);
		mesh.setVertices(vertices);
	}


	//--------------------------------------------------------------------------------------------
	//functii de cadru ---------------------------------------------------------------------------

	//functie chemata inainte de a incepe cadrul de desenare, o folosim ca sa updatam situatia scenei ( modelam/simulam scena)
    void notifyBeginFrame(){
    }
	//functia de afisare (lucram cu banda grafica)
	void notifyDisplayFrame(){

		angle += direction * 0.1f;
		if (angle >= 90.0f || angle <= -90.0f) {
			direction *= -1;
			angle = 0;
		}

		skydome_model_matrix = glm::rotate(skydome_model_matrix, 0.01f, glm::vec3(0.0f, 1.0f, 0.0f));
		
		//pe tot ecranul
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		//deseneaza obiectul cubemap /*

		cameraPosition = camera.getPosition();
		// vreau sfera sa fie mai in fata
		cameraPosition += glm::normalize(camera.getForward())*1.0f;
		new_view_matrix = glm::lookAt(cameraPosition, cameraPosition + glm::normalize(camera.getForward()), up);
		glUseProgram(shader_normal);
        glUniformMatrix4fv(glGetUniformLocation(shader_normal, "view_matrix"), 1, false, glm::value_ptr(camera.getViewMatrix()));
        glUniformMatrix4fv(glGetUniformLocation(shader_normal, "projection_matrix"), 1, false, glm::value_ptr(projection_matrix));
        glUniformMatrix4fv(glGetUniformLocation(shader_normal, "model_matrix"), 1, false, glm::value_ptr(skydome_model_matrix));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_sky);
        //glUniform1i(glGetUniformLocation(shader_normal, "textura_cubemap"), 1);
        skydome.Bind();
        skydome.Draw_Sphere();

		//glUseProgram(shader_cubemap);
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture_ground);
		//glUniform1i(glGetUniformLocation(shader_normal, "textura_cubemap"), 1
		glUniformMatrix4fv(glGetUniformLocation(shader_normal, "model_matrix"), 1, false, glm::value_ptr(glm::mat4(1)));
		ground.Bind();
		ground.Draw_Triangle_Fan();
		//draw_circle(600, 200, texture_sky);
		//sphere.draw(0, 0, -5);

        
        //deseneaza obiectul reflectant
        /*glUseProgram(shader_cubemap);
        glUniformMatrix4fv(glGetUniformLocation(shader_cubemap, "view_matrix"), 1, false, glm::value_ptr(camera.getViewMatrix()));
        glUniformMatrix4fv(glGetUniformLocation(shader_cubemap, "projection_matrix"), 1, false, glm::value_ptr(projection_matrix));
        glUniformMatrix4fv(glGetUniformLocation(shader_cubemap, "model_matrix"), 1, false, glm::value_ptr(glm::mat4(1)));
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture_sky);
        glUniform1i(glGetUniformLocation(shader_cubemap, "textura_cubemap"), texture_sky);
        glUniform3f(glGetUniformLocation(shader_cubemap, "camera_position"), camera.getPosition().x, camera.getPosition().y, camera.getPosition().z);
        //skydome.Bind();
		//skydome.Draw();
		*/

	}
	//functie chemata dupa ce am terminat cadrul de desenare (poate fi folosita pt modelare/simulare)
	void notifyEndFrame(){}
	//functei care e chemata cand se schimba dimensiunea ferestrei initiale
	void notifyReshape(int width, int height, int previos_width, int previous_height){
		//reshape
		if(height==0) height=1;
		float aspect = (float)width / (float)height;
		glViewport(0,0,width,height);
		projection_matrix = glm::perspective(75.0f, aspect,0.1f, 10000.0f);
	}


	//--------------------------------------------------------------------------------------------
	//functii de input output --------------------------------------------------------------------
	
	//tasta apasata
	void notifyKeyPressed(unsigned char key_pressed, int mouse_x, int mouse_y){
		if(key_pressed == 27) lab::glut::close();	//ESC inchide glut si 
        if (key_pressed == 32) {
            //SPACE reincarca shaderul si recalculeaza locatiile (offseti/pointeri)
            glDeleteProgram(shader_cubemap);
            glDeleteProgram(shader_normal);
            shader_normal = lab::loadShader("shadere/normal_vertex.glsl", "shadere/normal_fragment.glsl");
            shader_cubemap = lab::loadShader("shadere/cubemap_vertex.glsl", "shadere/cubemap_fragment.glsl");
        }
        if (key_pressed == 'w') camera.translateForward(5.0f);
        if (key_pressed == 'a') camera.translateRight(-5.0f);
        if (key_pressed == 's') camera.translateForward(-5.0f);
        if (key_pressed == 'd') camera.translateRight(5.0f);
        if (key_pressed == 'q') camera.rotateFPSoY(1.0f);
        if (key_pressed == 'e') camera.rotateFPSoY(-1.0f);
        if (key_pressed == 'r') camera.translateUpword(1.0f);
        if (key_pressed == 'f') camera.translateUpword(-1.0f);
	}
	//tasta ridicata
	void notifyKeyReleased(unsigned char key_released, int mouse_x, int mouse_y){	}
	//tasta speciala (up/down/F1/F2..) apasata
	void notifySpecialKeyPressed(int key_pressed, int mouse_x, int mouse_y){
		if(key_pressed == GLUT_KEY_F1) lab::glut::enterFullscreen();
		if(key_pressed == GLUT_KEY_F2) lab::glut::exitFullscreen();
	}
	//tasta speciala ridicata
	void notifySpecialKeyReleased(int key_released, int mouse_x, int mouse_y){}
	//drag cu mouse-ul
	void notifyMouseDrag(int mouse_x, int mouse_y){ }
	//am miscat mouseul (fara sa apas vreun buton)
	void notifyMouseMove(int mouse_x, int mouse_y){ }
	//am apasat pe un boton
	void notifyMouseClick(int button, int state, int mouse_x, int mouse_y){ }
	//scroll cu mouse-ul
	void notifyMouseScroll(int wheel, int direction, int mouse_x, int mouse_y){ }

};

int main(){
	//initializeaza GLUT (fereastra + input + context OpenGL)
	lab::glut::WindowInfo window(std::string("lab 4 - cubemaps"),800,600,100,100,true);
	lab::glut::ContextInfo context(3,3,false);
	lab::glut::FramebufferInfo framebuffer(true,true,true,true);
	lab::glut::init(window,context, framebuffer);

	//initializeaza GLEW (ne incarca functiile openGL, altfel ar trebui sa facem asta manual!)
	glewExperimental = true;
	glewInit();
	std::cout<<"GLEW:initializare"<<std::endl;

	//creem clasa noastra si o punem sa asculte evenimentele de la GLUT
	//DUPA GLEW!!! ca sa avem functiile de OpenGL incarcate inainte sa ii fie apelat constructorul (care creeaza obiecte OpenGL)
	Laborator mylab;
	lab::glut::setListener(&mylab);

	//taste
    std::cout << "Taste:" << std::endl << "\tESC ... iesire" << std::endl << "\tSPACE ... reincarca shadere" << std::endl;

	//run
	lab::glut::run();

	return 0;
}