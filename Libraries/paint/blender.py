import arm.material.mat_state as mat_state
import arm.material.make_mesh

def register():
	arm.material.make_mesh.write_material_attribs = write_material_attribs
	arm.material.mat_utils.add_mesh_contexts.append('voxel') # Defaults to make_ao
	arm.write_data.add_compiledglsl = """
		const ivec3 voxelgiResolution = ivec3(256, 256, 256);
		const vec3 voxelgiHalfExtents = vec3(2.0, 2.0, 2.0);
	"""

def write_material_attribs(con, frag):

	if mat_state.material.name != 'Material':
		return False

	con.add_elem('tex', 2)

	frag.write('vec3 basecol;')
	frag.write('float roughness;')
	frag.write('float metallic;')
	frag.write('float occlusion;')
	frag.write('float opacity;')

	frag.add_uniform('sampler2D texpaint')
	frag.write('basecol = pow(texture(texpaint, texCoord).rgb, vec3(2.2));')

	frag.add_uniform('sampler2D texpaint_nor')
	frag.add_include('std/normals.glsl')
	frag.write('mat3 TBN = cotangentFrame(n, -vVec, texCoord);')
	frag.write('n = texture(texpaint_nor, texCoord).rgb * 2.0 - 1.0;')
	frag.write('n = normalize(TBN * normalize(n));')

	frag.add_uniform('sampler2D texpaint_pack')
	frag.write('vec4 pack = texture(texpaint_pack, texCoord);')
	frag.write('occlusion = pack.r;')
	frag.write('roughness = pack.g;')
	frag.write('metallic = pack.b;')

	return True
