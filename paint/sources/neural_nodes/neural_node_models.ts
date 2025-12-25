
type neural_node_model_t = {
	name?: string;
	memory?: string;
	size?: string;
	nodes?: string;
	urls?: string[];
	web?: string;
	license?: string;
};
let neural_node_models: neural_node_model_t[] = null;

function neural_node_models_init() {
	neural_node_models = [
		{
			name : "Stable Diffusion",
			memory : "4GB",
			size : "4.3GB",
			nodes : "Inpaint Image, Outpaint Image, Text to Image, Tile Image, Vary Image",
			urls : [ "https://huggingface.co/stable-diffusion-v1-5/stable-diffusion-v1-5/resolve/main/v1-5-pruned-emaonly.safetensors" ],
			web : "https://huggingface.co/stable-diffusion-v1-5/stable-diffusion-v1-5",
			license : "openrail"
		},
		{
			name : "Z-Image-Turbo",
			memory : "4GB",
			size : "6.7GB",
			nodes : "Text to Image",
			urls : [
				"https://huggingface.co/armory3d/z_image_turbo/resolve/main/Qwen3-4B-Instruct-2507-Q4_K_S.gguf",
				"https://huggingface.co/armory3d/z_image_turbo/resolve/main/ae.safetensors",
				"https://huggingface.co/armory3d/z_image_turbo/resolve/main/z_image_turbo-Q4_K.gguf"
			],
			web : "https://huggingface.co/armory3d/z_image_turbo",
			license : "apache-2.0"
		},
		{
			name : "Qwen Image",
			memory : "13GB",
			size : "16.9GB",
			nodes : "Text to Image",
			urls : [
				"https://huggingface.co/QuantStack/Qwen-Image-GGUF/resolve/main/Qwen_Image-Q4_K_S.gguf",
				"https://huggingface.co/QuantStack/Qwen-Image-GGUF/resolve/main/VAE/Qwen_Image-VAE.safetensors",
				"https://huggingface.co/unsloth/Qwen2.5-VL-7B-Instruct-GGUF/resolve/main/Qwen2.5-VL-7B-Instruct-Q4_K_S.gguf"
			],
			web : "https://huggingface.co/QuantStack/Qwen-Image-GGUF",
			license : "apache-2.0"
		},
		{
			name : "Qwen Image Edit",
			memory : "13GB",
			size : "18.3GB",
			nodes : "Edit Image, Inpaint Image, Outpaint Image, Tile Image, Vary Image",
			urls : [
				"https://huggingface.co/QuantStack/Qwen-Image-Edit-2509-GGUF/resolve/main/Qwen-Image-Edit-2509-Q4_K_S.gguf",
				"https://huggingface.co/QuantStack/Qwen-Image-GGUF/resolve/main/VAE/Qwen_Image-VAE.safetensors",
				"https://huggingface.co/unsloth/Qwen2.5-VL-7B-Instruct-GGUF/resolve/main/Qwen2.5-VL-7B-Instruct-Q4_K_S.gguf",
				"https://huggingface.co/unsloth/Qwen2.5-VL-7B-Instruct-GGUF/resolve/main/mmproj-F16.gguf"
			],
			web : "https://huggingface.co/QuantStack/Qwen-Image-Edit-2509-GGUF",
			license : "apache-2.0"
		},
		{
			name : "Wan",
			memory : "10GB",
			size : "21.3GB",
			nodes : "Text to Image",
			urls : [
				"https://huggingface.co/QuantStack/Wan2.2-T2V-A14B-GGUF/resolve/main/LowNoise/Wan2.2-T2V-A14B-LowNoise-Q4_K_S.gguf",
				"https://huggingface.co/QuantStack/Wan2.2-T2V-A14B-GGUF/resolve/main/HighNoise/Wan2.2-T2V-A14B-HighNoise-Q4_K_S.gguf",
				"https://huggingface.co/QuantStack/Wan2.2-T2V-A14B-GGUF/resolve/main/VAE/Wan2.1_VAE.safetensors",
				"https://huggingface.co/city96/umt5-xxl-encoder-gguf/resolve/main/umt5-xxl-encoder-Q4_K_S.gguf"
			],
			web : "https://huggingface.co/QuantStack/Wan2.2-T2V-A14B-GGUF",
			license : "apache-2.0"
		},
		{
			name : "Marigold",
			memory : "6GB",
			size : "13.7GB",
			nodes : "Image to Depth, Image to Normal Map Node, Image to PBR",
			urls : [
				"https://huggingface.co/armory3d/marigold-v1-1-gguf/resolve/main/marigold-depth-v1-1.q8_0.gguf",
				"https://huggingface.co/armory3d/marigold-v1-1-gguf/resolve/main/marigold-normals-v1-1.q8_0.gguf",
				"https://huggingface.co/armory3d/marigold-v1-1-gguf/resolve/main/marigold-iid-appearance-v1-1.q8_0.gguf",
				"https://huggingface.co/armory3d/marigold-v1-1-gguf/resolve/main/marigold-iid-lighting-v1-1.q8_0.gguf"
			],
			web : "https://huggingface.co/armory3d/marigold-v1-1-gguf",
			license : "openrail"
		},
		{
			name : "Real-ESRGAN",
			memory : "1GB",
			size : "0.07GB",
			nodes : "Upscale Image",
			urls : [ "https://huggingface.co/armory3d/Real-ESRGAN/resolve/main/RealESRGAN_x4plus.pth" ],
			web : "https://huggingface.co/armory3d/Real-ESRGAN",
			license : "bsd-3-clause"
		},

        {
			name : "Hunyuan3D",
			memory : "12GB",
			size : "12.6GB",
			nodes : "Image to 3D Mesh",
			urls : [ "https://huggingface.co/armory3d/hunyuan3d21_portable/resolve/main/Hunyuan3D_win64.tar" ],
			web : "https://huggingface.co/armory3d/hunyuan3d21_portable",
			license : "hunyuan3d"
		}
	];
	// https://huggingface.co/webui/stable-diffusion-inpainting/resolve/main/sd-v1-5-inpainting.safetensors
}
