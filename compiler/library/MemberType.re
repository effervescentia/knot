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
  | Generic_t(option(generic_type))
  | Nil_t
and generic_type =
  | Keyed_t(Hashtbl.t(string, member_ref))
  | Callable_t(list(member_ref), member_ref)
and member_ref = ref(eventual_type)
and member_rule =
  | HasProperty(string, member_ref)
  | HasCallSignature(list(member_ref), member_ref)
and eventual_type =
  | Unanalyzed
  | Synthetic(member_type) /* for early type enforcement */
  | Resolved(member_type);

type ctxl_promise('a) = ('a, ref(ref(eventual_type)));

let no_ctx = x => (x, ref(ref(Unanalyzed)));
