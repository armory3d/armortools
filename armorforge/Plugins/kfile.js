let project = new Project('plugins');

project.addFile('Sources/**');

project.addDefine('WITH_PLUGIN_EMBED');

resolve(project);
