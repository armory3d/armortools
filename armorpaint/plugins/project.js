let project = new Project('plugins');

// project.addFile('sources/**'); ////

project.addDefine('WITH_PLUGIN_EMBED');
project.addDefine('TINYUSDZ_NO_STB_IMAGE_IMPLEMENTATION');

project.flatten();
return project;
