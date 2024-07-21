let project = new Project('plugins');

project.addFile('sources/**');

project.addDefine('WITH_PLUGIN_EMBED');

if (platform === Platform.Windows) {
	project.addLib('sources/proc_texsynth/win32/texsynth');
}
else if (platform === Platform.Linux) {
	process.env.LIBRARY_PATH = project.basedir + "/sources/proc_texsynth/linux";
	project.addLib('texsynth');
}
else if (platform === Platform.OSX) {
	project.addLib('sources/proc_texsynth/macos/libtexsynth.a');
}

return project;

