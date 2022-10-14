#pragma once
#include "Core/Log/Log.h"

namespace DX12Framework
{

	enum class ShaderDataType
	{
		None = 0, 

		Float,
		Float2, 
		Float3, 
		Float4, 

		Mat3, 
		Mat4, 

		Int, 
		Int2, 
		Int3,
		Int4, 
		
		Bool
	};


	static INT32 ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Float:  return 4;
		case ShaderDataType::Float2: return 4 * 2;
		case ShaderDataType::Float3: return 4 * 3;
		case ShaderDataType::Float4: return 4 * 4;

		case ShaderDataType::Mat3:   return 4 * 3 * 3;
		case ShaderDataType::Mat4:   return 4 * 4 * 4;

		case ShaderDataType::Int:    return 4;
		case ShaderDataType::Int2:   return 4 * 2;
		case ShaderDataType::Int3:   return 4 * 3;
		case ShaderDataType::Int4:   return 4 * 4;

		case ShaderDataType::Bool:   return 1;
		}


		CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

	struct BufferElement
	{
		std::string name;
		ShaderDataType type;
		INT32 size;
		INT32 offset;
		bool normalised;

		BufferElement() {}

		BufferElement(ShaderDataType type, const std::string& name, bool norm = false)
			:
			name(name), type(type), size(ShaderDataTypeSize(type)), offset(0), normalised(norm)
		{
		
		}

		INT32 GetComponentCount() const
		{
			switch (type)
			{
				case ShaderDataType::Int:	   return 1;
				case ShaderDataType::Int2:	   return 2;
				case ShaderDataType::Int3:	   return 3;
				case ShaderDataType::Int4:	   return 4;

				case ShaderDataType::Mat3:	   return 3 * 3;
				case ShaderDataType::Mat4:	   return 4 * 4;

				case ShaderDataType::Float:    return 1;
				case ShaderDataType::Float2:   return 2;
				case ShaderDataType::Float3:   return 3;
				case ShaderDataType::Float4:   return 4;

				case ShaderDataType::Bool:     return 1;
			}
		}

	};

	class BufferLayout
	{
	public:

		BufferLayout() {}

		BufferLayout(const std::initializer_list<BufferElement>& element)
		: 
			elements(element)
		{
			CalculateOffsetAndStride();
		}

		BufferLayout(const std::vector<BufferElement>& element)
			:
			elements(element)
		{
			CalculateOffsetAndStride();
		}

		inline INT32 GetStride() const { return stride; }

		inline const std::vector<BufferElement>& GetElements() const { return elements; }

		std::vector<BufferElement>::iterator begin() { return elements.begin(); }
		std::vector<BufferElement>::iterator end() { return elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return elements.end(); }

	private:

		void CalculateOffsetAndStride()
		{
			INT32 offset = 0;
			stride = 0;

			for (auto& element : elements)
			{
				element.offset = offset;
				offset += element.size;
				stride += element.size;
			}
		}

	private:
		std::vector<BufferElement> elements;
		INT32 stride;
	};

	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() = default;

		virtual void Bind() const = 0;
		virtual void UnBind() const = 0;

		virtual void SetData(const void* data, INT32 size) = 0;

		virtual inline void SetLayout(const BufferLayout& layout) = 0;

		virtual inline const BufferLayout& GetLayout() const = 0;

		static RefPointer<VertexBuffer> Create(INT32 size);
		static RefPointer<VertexBuffer> Create(float* vertices, INT32 size);

	};

	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() = default;

		virtual void Bind() const = 0;

		virtual void UnBind() const = 0;

		virtual const INT32 GetCount() = 0;

		static RefPointer<IndexBuffer> Create(INT32* indices, INT32 size);
	};

}

