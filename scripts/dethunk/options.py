"""
Runtime Options
"""

import tomllib


class Options:
    base_dir = str()

    # functions
    remove_constructor_thunk = bool()
    remove_destructor_thunk = bool()
    remove_virtual_function_thunk = bool()

    # variables
    remove_virtual_table_pointer_thunk = bool()
    restore_static_variable = bool()
    restore_member_variable = bool()

    # others (for class member variable)

    # * only takes effect for TypedStorage, since the TypedStorage wrapper makes the full type unnecessary.
    fix_includes_for_member_variables = True
    # * template definitions cannot be generated for headergen and may be wrong. class sizes may be wrong if
    # * empty templates are used in TypedStorage.
    # * this option will erase the type of the empty template (convert to uchar[size]).
    fix_size_for_type_with_empty_template_class = True
    # * this option will and add sizeof & alignof static assertions to members. (only takes effect for TypedStorage)
    add_sizeof_alignof_static_assertions = True
    # * some types of template definitions are not accurate.
    erase_extra_invalid_types = True
    # * try to initialize each class dynamically to ensure that the generated debug information is complete.
    # * this will ENSURE that all classes have a default constructor.
    add_trivial_dynamic_initializer = True

    # others (for static class member viriable)

    # * in msvc, 'const' object must be initialized if not 'extern' (c2734)
    # * see also https://reviews.llvm.org/D45978
    fix_msvc_c2734 = True

    exclusion_list = {}

    def __init__(self, args):
        self.base_dir = args.path
        self.remove_constructor_thunk = args.remove_constructor_thunk
        self.remove_destructor_thunk = args.remove_destructor_thunk
        self.remove_virtual_function_thunk = args.remove_virtual_function_thunk
        self.remove_virtual_table_pointer_thunk = args.remove_virtual_table_pointer_thunk
        self.restore_static_variable = args.restore_static_variable
        self.restore_member_variable = args.restore_member_variable

        if args.preset_extract_names:
            self.apply_preset_extract_names()

        if args.preset_extract_types:
            self.apply_preset_extract_types()

        if args.all:
            self.set_all(True)

        # others
        self.judge_other_options()

        with open(args.exclusion_list, 'rb') as file:
            self.exclusion_list = tomllib.load(file)

    def set_function(self, opt: bool):
        self.remove_constructor_thunk = opt
        self.remove_destructor_thunk = opt
        self.remove_virtual_function_thunk = opt

    def set_variable(self, opt: bool):
        self.remove_virtual_table_pointer_thunk = opt
        self.restore_static_variable = opt
        self.restore_member_variable = opt

    def set_all(self, opt: bool):
        self.set_function(opt)
        self.set_variable(opt)

    def apply_preset_extract_names(self):
        self.remove_constructor_thunk = True
        self.remove_destructor_thunk = True
        self.remove_virtual_function_thunk = True
        self.remove_virtual_table_pointer_thunk = True
        self.restore_static_variable = True

    def apply_preset_extract_types(self):
        self.apply_preset_extract_names()
        self.restore_member_variable = True

    def judge_other_options(self):
        if not self.restore_member_variable:
            self.fix_includes_for_member_variables = False
            self.fix_size_for_type_with_empty_template_class = False
            self.add_sizeof_alignof_static_assertions = False
            self.erase_extra_invalid_types = False
            self.add_trivial_dynamic_initializer = False

        if not self.restore_static_variable:
            self.fix_msvc_c2734 = False

    def _check_base(self, data, con: str):
        if 'equal' in data:
            for e in data['equal']:
                if con == e:
                    return True
        if 'not_equal' in data:
            for ne in data['not_equal']:
                if con == ne:
                    return False
        if 'contains' in data:
            for c in data['contains']:
                if c in con:
                    return True
        if 'startswith' in data:
            for s in data['startswith']:
                if con.startswith(s):
                    return True
        if 'endswith' in data:
            for e in data['endswith']:
                if con.endswith(e):
                    return True
        return False

    def should_ignore_forward_decl(self, decl):
        return self._check_base(self.exclusion_list['forward_decl_translator']['ignored'], decl)

    def should_ignore_generate_dynamic_initializer(self, type_name):
        return self._check_base(
            self.exclusion_list['trivial_dynamic_initializer_generator']['ignored'], type_name
        )

    def should_erase_type(self, type_name):
        return self._check_base(self.exclusion_list['typeunwrapper']['erased'], type_name)

    def should_erase_type_dyninit(self, type_name):
        return self._check_base(self.exclusion_list['typeunwrapper']['dyninit_erased'], type_name)
