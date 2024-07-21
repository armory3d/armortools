let project = new Project('plugins');

project.addFile('sources/**');

project.addDefine('WITH_PLUGIN_EMBED');

return project;
