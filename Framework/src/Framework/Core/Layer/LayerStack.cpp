#include "Framework/cmpch.h"
#include "LayerStack.h"

namespace Foundation
{
	LayerStack::~LayerStack()
	{
		for (Layer* layer : Layers)
		{
			delete layer;
			layer = nullptr;
		}
	}

	void LayerStack::PushLayer(Layer* layer)
	{
		Layers.emplace(Layers.begin() + LayerOffset, layer);
		LayerOffset++;
	}

	void LayerStack::PushOverlay(Layer* overlay)
	{
		Layers.emplace_back(overlay);
	}

	void LayerStack::PopLayer(Layer* layer)
	{
		auto iterator = std::find(Layers.begin(), Layers.end(), layer);

		if (iterator != Layers.end())
		{
			Layers.erase(iterator);
			LayerOffset--;
		}
	}

	void LayerStack::PopOverlay(Layer* overlay)
	{
		auto iterator = std::find(Layers.begin(), Layers.end(), overlay);

		if (iterator != Layers.end())
		{
			Layers.erase(iterator);
		}
	}
}

