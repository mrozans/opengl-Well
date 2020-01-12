#include "TubeMesh.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <iostream>

TubeMesh::TubeMesh(): height(30.0f), radius(1.5f), wallThickness(0.2f), segments(DEFAULT_SEGMENTS_NUMBER) {}

void TubeMesh::initializeMeshVertices(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, GLenum& drawingMode)
{
	generateWalls(vertices);
	generateIndices(vertices, indices);

	//for (size_t i = 0; i < indices.size(); i++)
	//{
	//	std::cout << indices[i] << ", ";
	//	if (i % 3 == 2)
	//		std::cout << std::endl;
	//}

	drawingMode = GL_TRIANGLES;
}

void TubeMesh::generateWalls(std::vector<Vertex>& vertices)
{
	for (unsigned int slice = 0; slice <= layers; ++slice)
	{
		glm::vec3 sliceCenter = glm::vec3(0.0f, height / 2.0f - height / layers * slice, 0.0f);
		float uCoord = 0.5f + 0.5f / layers * slice;
		for (float angle = 0.0f; angle < 360.0f; angle += 360.0f / segments)
		{
			vertices.push_back(Vertex(sliceCenter + glm::vec3(cosf(angle * M_PI / 180.0f) * (radius), 0.0f,
				-sinf(angle * M_PI / 180.0f) * (radius)), color, glm::vec2(uCoord, 1.0f - angle / 360.0f)));
		}
		vertices.push_back(Vertex(sliceCenter + glm::vec3(cosf(0.0f) * (radius),
			0.0f, -sinf(0.0f) * (radius)), color, glm::vec2(uCoord, 0.0f)));
	}

	for (int slice = layers; slice >= 0; --slice)
	{
		glm::vec3 sliceCenter = glm::vec3(0.0f, height / 2.0f - height / layers * slice, 0.0f);
		float uCoord = 0.5f + 0.5f / layers * slice;
		for (float angle = 0.0f; angle < 360.0f; angle += 360.0f / segments)
		{
			vertices.push_back(Vertex(sliceCenter + glm::vec3(cosf(angle * M_PI / 180.0f) * (radius - wallThickness), 0.0f,
				-sinf(angle * M_PI / 180.0f) * (radius - wallThickness)), color, glm::vec2(uCoord, 1.0f - angle / 360.0f)));
		}
		vertices.push_back(Vertex(sliceCenter + glm::vec3(cosf(0.0f) * (radius - wallThickness),
			0.0f, -sinf(0.0f) * (radius - wallThickness)), color, glm::vec2(uCoord, 0.0f)));
	}
	//std::cout << vertices.size() << std::endl;

	glm::vec3 center = glm::vec3(0.0f, height / 2.0f, 0.0f);
	glm::vec2 textureCenter = glm::vec2(0.25f, 0.25f);

	for (int j = 0; j != 2; ++j)
	{
		for (int i = 0; i != 2; ++i)
		{
			for (float angle = 0.0f; angle < 360.0f; angle += 360.0f / segments)
			{
				vertices.push_back(Vertex(center + glm::vec3(cosf(angle * M_PI / 180.0f) * (radius - wallThickness * i), 0.0f,
					-sinf(angle * M_PI / 180.0f) * (radius - wallThickness * i)), color, textureCenter + glm::vec2(cosf(angle * M_PI / 180.0f) * (radius - wallThickness * i) / 4 / radius,
						-sinf(angle * M_PI / 180.0f) * (radius - wallThickness * i) / 4 / radius)));
			}
		}
		center.y -= height;
		textureCenter = glm::vec2(0.25f, 0.75f);
	}
	//std::cout << vertices.size() << std::endl;
}

void TubeMesh::generateIndices(std::vector<Vertex>& vertices, std::vector<GLuint>& indices)
{
	for (unsigned int l = 0; l <= layers * 2; ++l)
	{
		if (l == layers)
			continue;
		unsigned int verticesperlayer = segments + 1;
		for (unsigned int i = 0; i <= verticesperlayer - 2; ++i)
		{
			indices.push_back(i + l * verticesperlayer);
			indices.push_back(i + 1 + l * verticesperlayer);
			indices.push_back(i + verticesperlayer * (l + 1));
			indices.push_back(i + 1 + l * verticesperlayer);
			indices.push_back(i + verticesperlayer * (l + 1));
			indices.push_back(i + 1 + verticesperlayer * (l + 1));
		}
	}

	unsigned int offset = layers * 4 * (segments + 1);

	for (unsigned int k = 0; k < 4 ; k+=2)
	{
		for (unsigned int i = 0; i <= segments - 1; ++i)
		{
			indices.push_back(i % segments + k * segments + offset);
			indices.push_back(i != segments - 1 ? i % segments + 1 + k * segments + offset : i % segments + 1 + (k-1) * segments + offset);
			indices.push_back(i % segments + segments * (k + 1) + offset);
			indices.push_back((i + 1) % segments + k * segments + offset);
			indices.push_back(i % segments + segments * (k + 1) + offset);
			indices.push_back((i + 1) % segments + segments * (k + 1) + offset);
		}
	}

}