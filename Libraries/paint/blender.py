import arm.material.mat_utils
import arm.material.make_shader
import arm.material.mat_state as mat_state
import arm.material.make_mesh

def register():
	# arm.material.mat_utils.add_mesh_contexts = ['depth']
	arm.material.make_mesh.write_material_attribs = write_material_attribs

def write_material_attribs(con, frag):

	if mat_state.material.name != 'Material':
		return False

	con.add_elem('tex', 2)
	# con.add_elem('tang', 3)

	frag.write('vec3 basecol;')
	frag.write('float roughness;')
	frag.write('float metallic;')
	frag.write('float occlusion;')
	frag.write('float opacity;')

	frag.add_uniform('sampler2D texpaint')
	frag.write('basecol = pow(texture(texpaint, texCoord).rgb, vec3(2.2));')

	frag.add_uniform('sampler2D texpaint_nor')
	# frag.write('vec3 n = texture(texpaint_nor, texCoord).rgb * 2.0 - 1.0;')
	frag.add_include('../../Shaders/std/normals.glsl')
	frag.write('mat3 TBN = cotangentFrame(n, -vVec, texCoord);')
	frag.write('n = texture(texpaint_nor, texCoord).rgb * 2.0 - 1.0;')
	frag.write('n = normalize(TBN * normalize(n));')

	frag.add_uniform('sampler2D texpaint_pack')
	frag.write('vec4 pack = texture(texpaint_pack, texCoord);')
	frag.write('occlusion = pack.r;')
	frag.write('roughness = pack.g;')
	frag.write('metallic = pack.b;')
	
	# # TODO: Sample disp at neightbour points to calc normal
	# tese.add_uniform('sampler2D texpaint_pack')
	# tese.write('vec4 pack = texture(texpaint_pack, texCoord);')
	# tese.write('disp = pack.b * 0.05;')

	return True
