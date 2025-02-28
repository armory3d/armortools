let project = new Project("plugins");

project.add_cfiles("plugin_api.c");
project.add_cfiles("plugins.c");

return project;
