let project = new Project('plugins');

project.addExclude('.git/**');
project.addExclude('build/**');
project.addFile('Sources/**');

project.addDefine('WITH_PLUGIN_EMBED');
project.addDefine('TINYUSDZ_NO_STB_IMAGE_IMPLEMENTATION');

resolve(project);
