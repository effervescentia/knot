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
  | Any_t
  | Nil_t
and member_ref = ref(eventual_ctx(member_type))
and member_rule =
  | HasProperty(string, member_ref)
  | HasCallSignature(list(member_ref), member_ref)
and eventual_ctx('a) =
  | Unanalyzed
  | Synthetic(list(member_rule)) /* for type casting */
  | Resolved('a);

type ctxl_promise('a) = ('a, ref(ref(eventual_ctx(member_type))));

let no_ctx = x => (x, ref(ref(Unanalyzed)));
