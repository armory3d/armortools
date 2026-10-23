// node paint/tests/material-import.mjs [path/to/ArmorPaint.exe] [optional local FBX]
import {spawn} from 'node:child_process';
import {mkdir, writeFile} from 'node:fs/promises';
import path from 'node:path';
import {fileURLToPath} from 'node:url';
import assert from 'node:assert/strict';
const repo=path.resolve(path.dirname(fileURLToPath(import.meta.url)), '../..');
const exe=path.resolve(process.argv[2] || path.join(repo,'paint/build/out/ArmorPaint.exe'));
const dir=path.join(repo,'paint/build/tests',`materials-${Date.now()}`);
await mkdir(dir,{recursive:true});
const q=s=>JSON.stringify(s.replaceAll('\\','/'));
const fixture=path.join(dir,'materials.FBX');
const centers=[[0,-0.65,0],[-0.7,0.45,0],[0.7,0.45,0]];
const vertices=centers.flatMap(([x,y,z])=>[[-.3,-.3],[.3,-.3],[.3,.3],[-.3,.3]].flatMap(([dx,dy])=>[x+dx,y+dy,z]));
const uv=[0,0,1,0,1,1,0,0,1,1,0,1];
await writeFile(fixture,`; FBX 7.4.0 project file
FBXHeaderExtension: { FBXHeaderVersion: 1003
 FBXVersion: 7400
}
GlobalSettings: { Version: 1000
 Properties70: {
  P: "UpAxis", "int", "Integer", "", 1
  P: "UpAxisSign", "int", "Integer", "", 1
  P: "FrontAxis", "int", "Integer", "", 2
  P: "FrontAxisSign", "int", "Integer", "", 1
  P: "CoordAxis", "int", "Integer", "", 0
  P: "CoordAxisSign", "int", "Integer", "", 1
 }
}
Objects: {
${[0,1].map(lod=>`Geometry: ${100+lod}, "Geometry::Head_LOD${lod}", "Mesh" {
 Vertices: *${vertices.length} { a: ${vertices} }
 PolygonVertexIndex: *18 { a: 0,1,-3,0,2,-4,4,5,-7,4,6,-8,8,9,-11,8,10,-12 }
 LayerElementNormal: 0 {
  Version: 101
  MappingInformationType: "ByPolygonVertex"
  ReferenceInformationType: "Direct"
  Normals: *54 { a: ${Array(18).fill([0,0,1]).flat()} }
 }
 ${[0,1].map(channel=>`LayerElementUV: ${channel} {
  Version: 101
  Name: "UV${channel}"
  MappingInformationType: "ByPolygonVertex"
  ReferenceInformationType: "Direct"
  UV: *36 { a: ${Array(3).fill(channel===0 ? uv : uv.map(v=>v*0.5)).flat()} }
 }`).join('\n')}
 LayerElementMaterial: 0 {
  Version: 101
  MappingInformationType: "ByPolygon"
  ReferenceInformationType: "IndexToDirect"
  Materials: *6 { a: 0,0,1,1,2,2 }
 }
 ${[0,1].map(channel=>`Layer: ${channel} {
  Version: 100
  LayerElement: { Type: "LayerElementUV"
   TypedIndex: ${channel}
  }
  ${channel===0 ? `LayerElement: { Type: "LayerElementNormal"
   TypedIndex: 0
  }
  LayerElement: { Type: "LayerElementMaterial"
   TypedIndex: 0
  }` : ''}
 }`).join('\n')}
}
Model: ${200+lod}, "Model::Head_LOD${lod}", "Mesh" { Version: 232 }
`).join('\n')}
${['Skin','EyeLeft','EyeRight'].map((name,i)=>`Material: ${300+i}, "Material::${name}", "" { Version: 102
 ShadingModel: "phong"
}`).join('\n')}
}
Connections: {
${[0,1].map(lod=>` C: "OO", ${100+lod}, ${200+lod}
 C: "OO", ${200+lod}, 0
${[0,1,2].map(i=>` C: "OO", ${300+i}, ${200+lod}`).join('\n')}`).join('\n')}
}
`);
async function run(name, source) {
 const script=path.join(dir,name+'.c');
 await writeFile(script,'#include "global.h"\n'+source);
 const output=await new Promise((resolve,reject)=>{
  const p=spawn(exe,['--background','--script',script],{cwd:path.dirname(exe),windowsHide:true});
  let out='';p.stdout.on('data',b=>out+=b);p.stderr.on('data',b=>out+=b);
  const timer=setTimeout(()=>{p.kill();reject(Error('Timeout '+name+'\n'+out));},60000);
  p.on('error',e=>{clearTimeout(timer);reject(e);});
  p.on('exit',code=>{clearTimeout(timer);code===0?resolve(out):reject(Error(name+' exit '+code+'\n'+out));});
 });
 await writeFile(path.join(dir,name+'.log'),output);
 assert(!/FAIL|error:/i.test(output),output);assert(output.includes('PASS'),output);
 console.log(name,output.trim());
}
const common=`
int frame=0;
slot_layer_t *layer;
int painted[64];
mesh_object_t *part(char *material) {
 any_array_t *objects=script_get_mesh_objects();
 for(int i=0;i<objects->length;i++) {
  mesh_object_t *o=objects->buffer[i];
  if(o->data->paint_uvs!=NULL && string_equals(o->data->paint_uvs->material,material) && o->base->visible) return o;
 }
 return NULL;
}
void report() {
 any_array_t *objects=script_get_mesh_objects();int visible=0,total=0;
 for(int i=0;i<objects->length;i++) {
  mesh_object_t *o=objects->buffer[i];
  if(o->data->index_array->length==0)continue;
  total++;if(o->base->visible)visible++;
  if(o->data->paint_uvs->tile<1)printf("FAIL: missing material tile");
  object_t *parent=o->base->parent;
  if(parent==NULL || !string_equals(parent->name,o->data->paint_uvs->source_object))printf("FAIL: material lost source parent");
  vertex_array_t *tex=mesh_data_get_vertex_array(o->data,"tex");
  vertex_array_t *tex1=mesh_data_get_vertex_array(o->data,"tex1");
  if(tex1==NULL || tex->values->length!=tex1->values->length)printf("FAIL: missing second UV channel");
  for(int j=0;j<tex->values->length;j++)if(tex->values->buffer[j]!=0 && tex->values->buffer[j]!=32767)printf("FAIL: original UV changed");
 }
 if(total!=6 || visible!=3)printf("FAIL: LOD visibility/count %d %d",total,visible);
}
void counts(int stride) {
 for(int i=0;i<64;i++)painted[i]=0;
 buffer_t *p=script_get_texture_pixels(layer->texpaint);
 int width=layer->texpaint->width,height=layer->texpaint->height;
 for(int y=0;y<height;y++)for(int x=0;x<width;x++)if(p->buffer[(y*width+x)*4+3]>0)painted[(y*stride/height)*stride+x*stride/width]++;
}
void mark() {
 mesh_object_t *o=part("EyeLeft");
 if(o==NULL){printf("FAIL: missing eye");return;}
 vertex_array_t *p=mesh_data_get_vertex_array(o->data,"pos");
 vec4_t c;c.x=0;c.y=0;c.z=0;c.w=1;
 for(int i=0;i<p->values->length;i+=4){c.x+=p->values->buffer[i];c.y+=p->values->buffer[i+1];c.z+=p->values->buffer[i+2];}
 float scale=o->data->scale_pos/32767.0/(p->values->length/4);c.x*=scale;c.y*=scale;c.z*=scale;
 object_t *parent=o->base;
 while(parent!=NULL){transform_t *t=parent->transform;c.x*=t->scale.x;c.y*=t->scale.y;c.z*=t->scale.z;c=vec4_apply_quat(c,t->rot);c.x+=t->loc.x;c.y+=t->loc.y;c.z+=t->loc.z;parent=parent->parent;}
 script_paint_world(c.x,c.y,c.z);script_paint_end();
}
void setup() {
 layer=script_layer_new();script_get_context()->brush_radius=0.4;
 script_material_set_color(script_material_get_node("RGB"),0,0,1,0,0,1);script_material_update();
 transform_t *t=script_get_camera()->base->transform;t->loc.x=0;t->loc.y=-3;t->loc.z=0;t->rot=quat_from_euler(1.57079633,0,0);
 transform_build_matrix(t);camera_object_build_mat(script_get_camera());script_get_context()->ddirty=3;
}
`;
const saved=path.join(dir,'painted.arm');
const appended=path.join(dir,'appended.arm');
const plain=path.join(dir,'plain.OBJ');
await writeFile(plain,'o Plain\nv 5 0 0\nv 6 0 0\nv 5 1 0\nvt 0 0\nvt 1 0\nvt 0 1\nf 1/1 2/2 3/3\n');
await run('separation-symmetry-undo-save',common+`
void update(){frame++;
 if(frame==2)script_import_mesh(${q(fixture)},0,-1,-1);
 if(frame==10){report();setup();}
 if(frame==16)mark();
 if(frame==20){counts(2);if(painted[0]!=0||painted[1]<100||painted[2]!=0)printf("FAIL: single eye leaked %d %d %d",painted[0],painted[1],painted[2]);script_undo();}
 if(frame==24){counts(2);if(painted[0]+painted[1]+painted[2]!=0)printf("FAIL: undo");script_redo();}
 if(frame==28){counts(2);if(painted[1]<100||painted[0]+painted[2]!=0)printf("FAIL: redo");script_layer_clear(layer);script_get_context()->paint_object=part("EyeLeft");script_get_context()->sym_x=1;}
 if(frame==34)mark();
 if(frame==38){counts(2);if(painted[0]!=0||painted[1]<100||painted[2]<100)printf("FAIL: symmetry %d %d %d",painted[0],painted[1],painted[2]);project_filepath_set(${q(saved)});project_save(0);}
 if(frame==44){printf("PASS: materials, LODs, UVs, painting, symmetry, undo/redo and save");script_quit();}
}
void main(){script_set_tools(0);script_notify_on_update(update);}
`);
await run('manual-axes-y-symmetry',common+`
void update(){frame++;
 if(frame==2)script_import_mesh(${q(fixture)},0,2,0);
 if(frame==10){report();setup();transform_t *t=script_get_camera()->base->transform;t->loc.x=-3;t->loc.y=0;t->rot=quat_from_euler(1.57079633,0,-1.57079633);transform_build_matrix(t);camera_object_build_mat(script_get_camera());script_get_context()->paint_object=part("EyeLeft");script_get_context()->sym_y=1;}
 if(frame==16)mark();
 if(frame==22){counts(2);if(painted[0]!=0||painted[1]<100||painted[2]<100)printf("FAIL: Y symmetry after axis conversion %d %d %d",painted[0],painted[1],painted[2]);printf("PASS: Y symmetry with manual source axes");script_quit();}
}
void main(){script_set_tools(0);script_notify_on_update(update);}
`);
await run('reopen-second-uv-append',common+`
void update(){frame++;
 if(frame==2)script_project_open(${q(saved)});
 if(frame==12){report();layer=script_layer_at(1);counts(2);if(painted[0]!=0||painted[1]<100||painted[2]<100)printf("FAIL: reopened paint");script_layer_set(layer);script_layer_clear(layer);layer->uv_map=1;script_layer_update(layer);}
 if(frame==18)mark();
 if(frame==22){counts(2);if(painted[0]!=0||painted[1]<100||painted[2]!=0)printf("FAIL: second UV isolation %d %d %d",painted[0],painted[1],painted[2]);buffer_t *p=script_get_texture_pixels(layer->texpaint);int w=layer->texpaint->width;int wrong=0;for(int y=0;y<w;y++)for(int x=0;x<w;x++)if(p->buffer[(y*w+x)*4+3]>0&&(x>w*0.75||y<w*0.25))wrong++;if(wrong>0)printf("FAIL: stroke used first UV channel %d",wrong);}
 if(frame==26)script_append_mesh_skinned(${q(fixture)});
 if(frame==34){counts(3);if(painted[0]!=0||painted[1]<10||painted[2]!=0)printf("FAIL: append moved existing paint %d %d %d",painted[0],painted[1],painted[2]);int skins=0,visible=0;any_array_t *objects=script_get_mesh_objects();for(int i=0;i<objects->length;i++){mesh_object_t *o=objects->buffer[i];if(script_clip_frame_count(o->base)>0){skins++;if(o->base->visible)visible++;if(o->data->paint_uvs->tile<5)printf("FAIL: appended material overlaps existing tile");object_t *parent=o->base->parent;if(parent==NULL||!starts_with(parent->name,"Head_LOD"))printf("FAIL: appended LOD grouping");}}if(skins!=6||visible!=3)printf("FAIL: skinned material parts/visibility %d %d",skins,visible);}
 if(frame==38)script_append_mesh(${q(plain)});
 if(frame==44){counts(4);if(painted[0]!=0||painted[1]<10||painted[2]!=0)printf("FAIL: plain append moved paint");any_array_t *objects=script_get_mesh_objects();for(int i=0;i<objects->length;i++){mesh_object_t *o=objects->buffer[i];if(o->data->index_array->length>0&&o->data->paint_uvs->count!=10)printf("FAIL: atlas capacity not preserved");if(string_equals(o->base->name,"Plain")&&o->data->paint_uvs->tile!=10)printf("FAIL: plain append overlaps material");}project_filepath_set(${q(appended)});project_save(0);}
 if(frame==48){printf("PASS: reopen, distinct second UV, skinned/plain append and atlas migration");script_quit();}
}
void main(){script_set_tools(0);script_notify_on_update(update);}
`);
await run('reopen-appended',common+`
void update(){frame++;
 if(frame==2)script_project_open(${q(appended)});
 if(frame==12){layer=script_layer_at(1);counts(4);if(painted[0]!=0||painted[1]<10||painted[2]!=0)printf("FAIL: appended paint reopen");printf("PASS: appended atlas persists");script_quit();}
}
void main(){script_set_tools(0);script_notify_on_update(update);}
`);
if(process.argv[3]) {
 const local=path.resolve(process.argv[3]);
 await run('local-fbx-eye',`
int frame=0;slot_layer_t *layer;
void update(){frame++;
 if(frame==2)script_import_mesh(${q(local)},0,-1,-1);
 if(frame==12){layer=script_layer_new();script_get_context()->brush_radius=0.2;script_material_set_color(script_material_get_node("RGB"),0,0,1,0,0,1);script_material_update();}
 if(frame==24)script_mouse_click(790,590,0);
 if(frame==34){buffer_t *p=script_get_texture_pixels(layer->texpaint);int count=0,skin=0;int w=layer->texpaint->width;for(int y=0;y<w;y++)for(int x=0;x<w;x++)if(p->buffer[(y*w+x)*4+3]>0){count++;if(x<w/3&&y<w/3)skin++;}if(count==0||skin>0)printf("FAIL: eye stroke affected skin %d total %d",skin,count);script_screenshot(${q(path.join(dir,'local-eye.png'))});}
 if(frame==42){printf("PASS: local eye click does not paint skin");script_quit();}
}
void main(){script_set_tools(0);script_notify_on_update(update);}
`);
}
console.log('Results: '+dir);
