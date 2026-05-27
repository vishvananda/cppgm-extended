# Checked-in frontend source-set manifest for reduced-link frontend builds.
# Keep these object lists in sync with dev/src and frontend entrypoints.

FRONTEND_SOURCE_SET_TARGETS := abimangle pptoken posttoken ctrlexpr macro preproc recog nsdecl nsinit cy86 cppgm++ lowiropt lowir2cy86 lowir2native cpplink cppeh

# abimangle: shared object(s)
FRONTEND_OBJ_BASENAMES_abimangle := \
	abi_mangle \
	cpp_decl_bridge \
	cpp_decl_model \
	encoding \
	host_builtin_runtime \
	pack_parameter_analysis \
	parser_trace \
	posttokenizer \
	pptokenizer \
	qualified_name_parser \
	recog_parser \
	recog_token_buffer \
	recog_token_cursor \
	semantic_utils \
	source_location \
	template_angle_parser \
	text_intern \
	types

# pptoken: 2 shared object(s)
FRONTEND_OBJ_BASENAMES_pptoken := \
	encoding \
	pptokenizer

# posttoken: 5 shared object(s)
FRONTEND_OBJ_BASENAMES_posttoken := \
	encoding \
	posttokenizer \
	pptokenizer \
	source_location \
	types

# ctrlexpr: 4 shared object(s)
FRONTEND_OBJ_BASENAMES_ctrlexpr := \
	calculator \
	encoding \
	pptokenizer \
	types

# macro: 9 shared object(s)
FRONTEND_OBJ_BASENAMES_macro := \
	calculator \
	encoding \
	file_timing \
	macroizer \
	posttokenizer \
	pptokenizer \
	preprocessor \
	source_location \
	types

# preproc: 10 shared object(s)
FRONTEND_OBJ_BASENAMES_preproc := \
	calculator \
	encoding \
	file_timing \
	macroizer \
	posttokenizer \
	pptokenizer \
	preproc_output \
	preprocessor \
	source_location \
	types

# recog: 15 shared object(s)
FRONTEND_OBJ_BASENAMES_recog := \
	calculator \
	encoding \
	file_timing \
	macroizer \
	parser_trace \
	posttokenizer \
	pptokenizer \
	preprocessor \
	recog_parser \
	recog_token_buffer \
	recog_token_cursor \
	source_location \
	template_angle_parser \
	text_intern \
	types

# nsdecl: 83 shared object(s)
FRONTEND_OBJ_BASENAMES_nsdecl := \
	calculator \
	callsem_output \
	callsemantic \
	callsemantic/class_template_reference \
	callsemantic/constant_call_evaluation \
	callsemantic/constant_value_lookup \
	callsemantic/function_registry \
	callsemantic/memory_census \
	callsemantic/nothrow_analysis \
	callsemantic/source_location_utils \
	callsemantic/template_body_checks \
	callsemantic/template_declaration_collector \
	callsemantic/template_source_utils \
	callsemantic/type_registry \
	callsemantic/type_trait_analysis \
	callsemantic_lookup \
	callsemantic_phase_bridge \
	callsemantic_text \
	constant_value \
	constexpr_eval \
	constructor_lifecycle_service \
	cpp_decl_ast \
	cpp_decl_bridge \
	cpp_decl_model \
	cppast_dump \
	cppast_parser \
	encoding \
	file_timing \
	macroizer \
	nsdecl_semantic \
	output_requirement_engine \
	pack_parameter_analysis \
	parser_trace \
	posttokenizer \
	pptokenizer \
	preprocessor \
	qualified_name_parser \
	recog_parser \
	recog_token_buffer \
	recog_token_cursor \
	rtti_names \
	semantic_builtins \
	semantic_cache \
	semantic_class_model \
	semantic_consteval \
	semantic_conversion \
	semantic_declaration \
	semantic_dependent_type \
	semantic_expression \
	semantic_fallback_audit \
	semantic_hotspot \
	semantic_lifetime \
	semantic_lookup \
	semantic_metrics \
	semantic_model \
	semantic_output \
	semantic_overload \
	semantic_parameter_recovery \
	semantic_scope_mutation \
	semantic_statement \
	semantic_template_class \
	semantic_template_function \
	semantic_template_output_policy \
	semantic_template_variable \
	semantic_trace \
	semantic_utils \
	source_location \
	symbol_linkage \
	template_angle_parser \
	text_intern \
	template_api \
	template_argument_semantics \
	template_audit \
	template_binding \
	template_binding_ops \
	template_decl_ast \
	template_function_deduction_api \
	template_function_signature \
	template_instantiation \
	template_model \
	template_resolution \
	template_resolution_ops \
	template_scope \
	template_selection \
	template_specialization_ops \
	template_signature_ops \
	template_specialization \
	witness_api \
	template_type_ops \
	types

# nsinit: 84 shared object(s)
FRONTEND_OBJ_BASENAMES_nsinit := \
	calculator \
	callsem_output \
	callsemantic \
	callsemantic/class_template_reference \
	callsemantic/constant_call_evaluation \
	callsemantic/constant_value_lookup \
	callsemantic/function_registry \
	callsemantic/memory_census \
	callsemantic/nothrow_analysis \
	callsemantic/source_location_utils \
	callsemantic/template_body_checks \
	callsemantic/template_declaration_collector \
	callsemantic/template_source_utils \
	callsemantic/type_registry \
	callsemantic/type_trait_analysis \
	callsemantic_lookup \
	callsemantic_phase_bridge \
	callsemantic_text \
	constant_value \
	constexpr_eval \
	constructor_lifecycle_service \
	cpp_decl_ast \
	cpp_decl_bridge \
	cpp_decl_model \
	cppast_dump \
	cppast_parser \
	encoding \
	file_timing \
	macroizer \
	nsinit_image \
	nsinit_semantic \
	output_requirement_engine \
	pack_parameter_analysis \
	parser_trace \
	posttokenizer \
	pptokenizer \
	preprocessor \
	qualified_name_parser \
	recog_parser \
	recog_token_buffer \
	recog_token_cursor \
	rtti_names \
	semantic_builtins \
	semantic_cache \
	semantic_class_model \
	semantic_consteval \
	semantic_conversion \
	semantic_declaration \
	semantic_dependent_type \
	semantic_expression \
	semantic_fallback_audit \
	semantic_hotspot \
	semantic_lifetime \
	semantic_lookup \
	semantic_metrics \
	semantic_model \
	semantic_output \
	semantic_overload \
	semantic_parameter_recovery \
	semantic_scope_mutation \
	semantic_statement \
	semantic_template_class \
	semantic_template_function \
	semantic_template_output_policy \
	semantic_template_variable \
	semantic_trace \
	semantic_utils \
	source_location \
	symbol_linkage \
	template_angle_parser \
	text_intern \
	template_api \
	template_argument_semantics \
	template_audit \
	template_binding \
	template_binding_ops \
	template_decl_ast \
	template_function_deduction_api \
	template_function_signature \
	template_instantiation \
	template_model \
	template_resolution \
	template_resolution_ops \
	template_scope \
	template_selection \
	template_specialization_ops \
	template_signature_ops \
	template_specialization \
	witness_api \
	template_type_ops \
	types

# cy86: 17 shared object(s)
FRONTEND_OBJ_BASENAMES_cy86 := \
	calculator \
	cy86_compiler \
	cy86_native_backend \
	cy86_parser \
	elf_writer \
	encoding \
	file_timing \
	macho_writer \
	macroizer \
	native_format \
	posttokenizer \
	pptokenizer \
	preprocessor \
	source_location \
	types \
	x86_assembler

# cppgm++: 111 shared object(s)
FRONTEND_OBJ_BASENAMES_cppgm++ := \
	calculator \
	callsem_output \
	callsemantic \
	callsemantic/class_template_reference \
	callsemantic/constant_call_evaluation \
	callsemantic/constant_value_lookup \
	callsemantic/function_registry \
	callsemantic/memory_census \
	callsemantic/nothrow_analysis \
	callsemantic/source_location_utils \
	callsemantic/template_body_checks \
	callsemantic/template_declaration_collector \
	callsemantic/template_source_utils \
	callsemantic/type_registry \
	callsemantic/type_trait_analysis \
	callsemantic_lookup \
	callsemantic_phase_bridge \
	callsemantic_text \
	cli_batch_frontend \
	constant_value \
	constexpr_eval \
	constructor_lifecycle_service \
	cpp_batch_frontend \
	cpp_decl_ast \
	cpp_decl_bridge \
	cpp_decl_model \
	cpp_driver_frontend \
	cpp_text_generators \
	cpp_tool_cli \
	cpp_toolchain \
	cppast_dump \
	cppast_parser \
	cy86_compiler \
	cy86_native_backend \
	cy86_parser \
	eh_runtime \
	elf_writer \
	encoding \
	file_timing \
	host_builtin_runtime \
	host_eh_object_sections \
	lowir_internal \
	lowir_machine_ir \
	lowir_object_backend \
	machine_ir_optimizer \
	lowir_optimizer \
	lowirgensemantic \
	machine_linker \
	machine_object \
	macho_writer \
	macroizer \
	native_format \
	optimization_level \
	output_requirement_engine \
	pack_parameter_analysis \
	parser_trace \
	posttokenizer \
	pptokenizer \
	preproc_output \
	preprocessor \
	qualified_name_parser \
	recog_parser \
	recog_token_buffer \
	recog_token_cursor \
	rtti_names \
	runtime_symbol_policy \
	semantic_builtins \
	semantic_cache \
	semantic_class_model \
	semantic_consteval \
	semantic_conversion \
	semantic_declaration \
	semantic_dependent_type \
	semantic_expression \
	semantic_fallback_audit \
	semantic_hotspot \
	semantic_lifetime \
	semantic_lookup \
	semantic_metrics \
	semantic_model \
	semantic_output \
	semantic_overload \
	semantic_parameter_recovery \
	semantic_scope_mutation \
	semantic_statement \
	semantic_template_class \
	semantic_template_function \
	semantic_template_output_policy \
	semantic_template_variable \
	semantic_trace \
	semantic_utils \
	source_location \
	symbol_linkage \
	template_angle_parser \
	text_intern \
	template_api \
	template_argument_semantics \
	template_audit \
	template_binding \
	template_binding_ops \
	template_decl_ast \
	template_function_deduction_api \
	template_function_signature \
	template_instantiation \
	template_model \
	template_resolution \
	template_resolution_ops \
	template_scope \
	template_selection \
	template_specialization_ops \
	template_signature_ops \
	template_specialization \
	template_text_output \
	template_witness_renderer \
	template_type_ops \
	types \
	typesemantic \
	witness_text \
	witness_api \
	x86_assembler

# lowiropt: 22 shared object(s)
FRONTEND_OBJ_BASENAMES_lowiropt := \
	cli_batch_frontend \
	cpp_decl_bridge \
	cpp_decl_model \
	encoding \
	host_builtin_runtime \
	lowir_internal \
	lowir_optimizer \
	optimization_level \
	pack_parameter_analysis \
	parser_trace \
	posttokenizer \
	pptokenizer \
	qualified_name_parser \
	recog_parser \
	recog_token_buffer \
	recog_token_cursor \
	semantic_utils \
	source_location \
	symbol_linkage \
	template_angle_parser \
	text_intern \
	types

# lowir2cy86: 31 shared object(s)
FRONTEND_OBJ_BASENAMES_lowir2cy86 := \
	calculator \
	cpp_decl_bridge \
	cpp_decl_model \
	cy86_compiler \
	cy86_native_backend \
	cy86_parser \
	eh_runtime \
	elf_writer \
	encoding \
	file_timing \
	lowir_backend \
	lowir_cy86_backend \
	lowir_internal \
	macho_writer \
	macroizer \
	native_format \
	pack_parameter_analysis \
	parser_trace \
	posttokenizer \
	pptokenizer \
	preprocessor \
	qualified_name_parser \
	recog_parser \
	recog_token_buffer \
	recog_token_cursor \
	semantic_utils \
	source_location \
	symbol_linkage \
	template_angle_parser \
	text_intern \
	types \
	x86_assembler

# lowir2native: 39 shared object(s)
FRONTEND_OBJ_BASENAMES_lowir2native := \
	calculator \
	cli_batch_frontend \
	cpp_decl_bridge \
	cpp_decl_model \
	cy86_compiler \
	cy86_native_backend \
	cy86_parser \
	eh_runtime \
	elf_writer \
	encoding \
	file_timing \
	host_eh_object_sections \
	lowir_driver_frontend \
	lowir_internal \
	lowir_machine_ir \
	lowir_object_backend \
	lowir_tool_cli \
	machine_ir \
	machine_ir_optimizer \
	machine_linker \
	machine_object \
	macho_writer \
	macroizer \
	native_format \
	optimization_level \
	pack_parameter_analysis \
	parser_trace \
	posttokenizer \
	pptokenizer \
	preprocessor \
	qualified_name_parser \
	recog_parser \
	recog_token_buffer \
	recog_token_cursor \
	runtime_symbol_policy \
	semantic_utils \
	source_location \
	symbol_linkage \
	template_angle_parser \
	text_intern \
	types \
	x86_assembler

# cpplink: 39 shared object(s)
FRONTEND_OBJ_BASENAMES_cpplink := \
	calculator \
	cli_batch_frontend \
	cpp_decl_bridge \
	cpp_decl_model \
	cy86_compiler \
	cy86_native_backend \
	cy86_parser \
	eh_runtime \
	elf_writer \
	encoding \
	file_timing \
	host_eh_object_sections \
	lowir_driver_frontend \
	lowir_internal \
	lowir_machine_ir \
	lowir_object_backend \
	lowir_tool_cli \
	machine_ir \
	machine_ir_optimizer \
	machine_linker \
	machine_object \
	macho_writer \
	macroizer \
	native_format \
	optimization_level \
	pack_parameter_analysis \
	parser_trace \
	posttokenizer \
	pptokenizer \
	preprocessor \
	qualified_name_parser \
	recog_parser \
	recog_token_buffer \
	recog_token_cursor \
	runtime_symbol_policy \
	semantic_utils \
	source_location \
	symbol_linkage \
	template_angle_parser \
	text_intern \
	types \
	x86_assembler

# cppeh: 39 shared object(s)
FRONTEND_OBJ_BASENAMES_cppeh := \
	calculator \
	cli_batch_frontend \
	cpp_decl_bridge \
	cpp_decl_model \
	cy86_compiler \
	cy86_native_backend \
	cy86_parser \
	eh_runtime \
	elf_writer \
	encoding \
	file_timing \
	host_eh_object_sections \
	lowir_driver_frontend \
	lowir_internal \
	lowir_machine_ir \
	lowir_object_backend \
	lowir_tool_cli \
	machine_ir \
	machine_ir_optimizer \
	machine_linker \
	machine_object \
	macho_writer \
	macroizer \
	native_format \
	optimization_level \
	pack_parameter_analysis \
	parser_trace \
	posttokenizer \
	pptokenizer \
	preprocessor \
	qualified_name_parser \
	recog_parser \
	recog_token_buffer \
	recog_token_cursor \
	runtime_symbol_policy \
	semantic_utils \
	source_location \
	symbol_linkage \
	template_angle_parser \
	text_intern \
	types \
	x86_assembler
