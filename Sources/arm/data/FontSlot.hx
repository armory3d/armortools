package arm.data;

import haxe.Json;
import kha.Image;
import kha.Blob;
import zui.Nodes;
import iron.data.Data;

class FontSlot {
	public var image: Image = null; // 200px
	public var previewReady = false;
	public var id = 0;
	public var font: kha.Font;
	public var name: String;

	public function new(name: String, font: kha.Font) {
		for (slot in Project.fonts) if (slot.id >= id) id = slot.id + 1;
		this.name = name;
		this.font = font;
	}
}
