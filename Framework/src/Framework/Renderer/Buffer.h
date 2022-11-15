#pragma once
#include "Framework/Core/Log/Log.h"
#include "Framework/Core/Time/AppTimeManager.h"
#include "Framework/Camera/MainCamera.h"


namespace Engine
{

	class GraphicsContext;
	class RenderItem;
	class FrameResource;

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
		UINT Size;
		UINT Offset;
		bool Normalised;

		BufferElement() = default;

		BufferElement(std::string&& name, ShaderDataType type, bool normalised = false)
			:
			Name(std::move(name)), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalised(normalised)
		{
		
		}

		BufferElement(std::string&& name, ShaderDataType type, INT32 alignedByteOffset, INT32 semanticIndex = 0, bool normalised = false)
			:
			Name(std::move(name)), Type(type), Size(ShaderDataTypeSize(type)), Offset(alignedByteOffset), Normalised(normalised)
		{

		}

		UINT GetComponentCount() const
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

		UINT GetStride() const { return Stride; }

		const std::vector<BufferElement>& GetElements() const { return Elements; }

		std::vector<BufferElement>::iterator begin() { return Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return Elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return Elements.end(); }

	private:

		void CalculateOffsetAndStride()
		{
			UINT offset = 0;
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
		UINT Stride;
	};

	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() = default;

		virtual void Bind() const = 0;
		virtual void UnBind() const = 0;

		virtual void SetData(GraphicsContext* graphicsContext, const void* data, INT32 size) = 0;

		virtual void SetLayout(const BufferLayout& layout) = 0;

		virtual const BufferLayout& GetLayout() const = 0;

		static RefPointer<VertexBuffer> Create(GraphicsContext* const graphicsContext, UINT size, UINT vertexCount);
		static RefPointer<VertexBuffer> Create(GraphicsContext* const graphicsContext, const void* vertices, UINT size, UINT vertexCount);

	};

	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() = default;

		virtual void Bind() const = 0;

		virtual void UnBind() const = 0;

		virtual const INT32 GetCount() = 0;

		static RefPointer<IndexBuffer> Create(GraphicsContext* const graphicsContext, UINT16* indices, UINT size, UINT indexCount);
	};


	class UploadBufferManager
	{
	public:

		virtual ~UploadBufferManager() = default;

		virtual void Bind() const = 0;

		virtual void UnBind() const = 0;

		virtual void Update(const MainCamera& camera, const AppTimeManager& appTimeManager) = 0;

		virtual void UpdateConstantBuffer(std::vector<RefPointer<RenderItem>> items) = 0;

		virtual const INT32 GetCount() const = 0;

		virtual void CreateMainPassConstBuffer(GraphicsContext* graphicsContext, UINT passCount, UINT objectCount) = 0;

		static RefPointer<UploadBufferManager> Create(GraphicsContext* const graphicsContext, FrameResource* const frameResources, UINT count, bool isConstant, UINT frameResourceCount, UINT renderItemsCount);

	};

}

