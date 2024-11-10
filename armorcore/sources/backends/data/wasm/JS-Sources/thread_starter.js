function create_thread(func) {

}

onmessage = function(e) {
  console.log('onmessage');
  
  const mod = e.data.mod;
  const memory = e.data.memory;
  const func = e.data.func;

  const importObject = {
    env: { memory },
    imports: {
      imported_func: arg => console.log('thread: ' + arg),
      create_thread
    }
  };

  WebAssembly.instantiate(mod, importObject).then((instance) => {
    console.log('Running thread');
    instance.exports[func]();
  });
};
