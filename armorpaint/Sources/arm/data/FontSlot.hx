package arm.data;

import kha.Image;
import kha.Font;

class FontSlot {
	public var image: Image = null; // 200px
	public var previewReady = false;
	public var id = 0;
	public var font: Font;
	public var name: String;
	public var file: String;

	public function new(name: String, font: Font, file = "") {
		for (slot in Project.fonts) if (slot.id >= id) id = slot.id + 1;
		this.name = name;
		this.font = font;
		this.file = file;
	}
}
