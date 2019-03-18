type member_type =
  | Number_t
  | String_t
  | Boolean_t
  | Array_t(member_ref)
  | Object_t(Hashtbl.t(string, member_ref))
  | Function_t(list(member_ref), member_ref)
  | JSX_t
  | View_t
  | State_t
  | Style_t
  | Module_t(
      list(string),
      Hashtbl.t(string, member_ref),
      option(member_ref),
    )
  /* Generic(None) for "any", otherwise can only be keyed or callable */
  | Generic_t(option(generic_type))
  | Nil_t
and generic_type =
  /* could be a Module or an Object */
  | Keyed_t(Hashtbl.t(string, member_ref))
  /* could be a function (or a view???) */
  | Callable_t(list(member_ref), member_ref)
and member_ref = ref(eventual_type)
and eventual_type =
  | Inferred(member_type)
  | Declared(member_type);

type ctxl_promise('a) = ('a, ref(option(member_ref)));

let no_ctx = x => (x, ref(None));
