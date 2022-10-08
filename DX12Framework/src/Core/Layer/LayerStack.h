#pragma once
#include "Layer.h"


namespace DX12Framework
{
	// @brief The layer stack is a custom structure which stores the application's
	//		  frontend layers.
	//		  Additionally, the layer system tracks overlays, such as ImGui which handles
	//		  user interface rendering. 

	class LayerStack
	{
	public:
		LayerStack() = default;
		~LayerStack();

		// @brief Pushes a layer onto the stack.
		// @param[in] Takes a pointer to a layer allocated on the heap.
		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		// @brief Pops the last inserted layer and adjusts the layer offset accordingly.
		// @param[in] Takes a pointer to a layer allocated on the heap.
		void PopLayer(Layer* layer);
		void PopOverlay(Layer* layer);

		std::vector<Layer*>::iterator Begin() { return Layers.begin(); }
		std::vector<Layer*>::iterator End() { return Layers.end(); }

	private:

		std::vector<Layer*> Layers;
		UINT32 LayerOffset = 0;
	};
}


