/// <reference path='./Translator.ts'/>

let tr = Translator.tr;
let _tr = Translator._tr;

let array_f32 = (ar: f32[]): Float32Array => {
    let res = new Float32Array(ar.length);
    for (let i = 0; i < ar.length; ++i) res[i] = ar[i];
    return res;
}
