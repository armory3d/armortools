let project = new Project('plugins');

project.addFile('Sources/**');

project.addDefine('WITH_PLUGIN_EMBED');

if (platform === Platform.Windows) {
	project.addLib('Sources/proc_texsynth/win32/texsynth');
}
else if (platform === Platform.Linux) {
	process.env.LIBRARY_PATH = project.basedir + "/Sources/proc_texsynth/linux";
	project.addLib('texsynth');
}
else if (platform === Platform.OSX) {
	project.addLib('Sources/proc_texsynth/macos/libtexsynth.a');
}

resolve(project);

