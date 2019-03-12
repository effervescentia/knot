type member_type =
  | Number_t
  | String_t
  | Boolean_t
  | Array_t(member_type)
  | Object_t(Hashtbl.t(string, member_type))
  | Function_t(list(member_type), member_type)
  | JSX_t
  | View_t
  | State_t
  | Style_t
  | Module_t(
      list(string),
      Hashtbl.t(string, member_type),
      option(member_type),
    )
  | Any_t(int)
  | Nil_t;
