#pragma once
#include "Framework/Core/Log/Log.h"

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
		std::string Name;
		ShaderDataType Type;
		INT32 Size;
		INT32 Offset;
		bool Normalised;

		BufferElement() = default;

		BufferElement(ShaderDataType type, const std::string& name, bool normalised = false)
			:
			Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalised(normalised)
		{
		
		}

		INT32 GetComponentCount() const
		{
			switch (Type)
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

		BufferLayout() = default;

		BufferLayout(const std::initializer_list<BufferElement>& element)
		: 
			Elements(element)
		{
			CalculateOffsetAndStride();
		}

		BufferLayout(const std::vector<BufferElement>& element)
			:
			Elements(element)
		{
			CalculateOffsetAndStride();
		}

		INT32 GetStride() const { return Stride; }

		const std::vector<BufferElement>& GetElements() const { return Elements; }

		std::vector<BufferElement>::iterator begin() { return Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return Elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return Elements.end(); }

	private:

		void CalculateOffsetAndStride()
		{
			INT32 offset = 0;
			Stride = 0;

			for (auto& element : Elements)
			{
				element.Offset = offset;
				offset += element.Size;
				Stride += element.Size;
			}
		}

	private:
		std::vector<BufferElement> Elements;
		INT32 Stride;
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

