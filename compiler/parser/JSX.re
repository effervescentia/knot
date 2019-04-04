open Core;

/* </abc> */
let close_tag = name =>
  M.jsx_open_end >> M.exact_identifier(name) >> M.r_chev;

let rec expr = x => fragment(x) <|> element(x)
/* TODO: should figure out why `fragment` cannot appear in `nested_expr` */
and nested_expr = x => eval_node(x) <|> element(x)
and nested_exprs = x => nested_expr(x) |> many
and element = x =>
  M.l_chev
  >> M.identifier
  >>= (
    name =>
      jsx_tags
      |= []
      >>= (
        tags =>
          jsx_property(x)
          |> many
          >>= (
            props =>
              container_element(x, name)
              <|> self_closing_element
              ==> (children => Element(name, tags, props, children))
          )
      )
  )
and container_element = (x, name) =>
  M.r_chev >> nested_exprs(x) << close_tag(name)
and self_closing_element = M.jsx_self_close >> return([])
and fragment = x =>
  M.jsx_start_frag
  >> nested_exprs(x)
  << M.jsx_end_frag
  ==> (children => Fragment(children))
and eval_node = x => M.braces(x) ==> no_ctx % (ex => EvalNode(ex))
/* figure out how to do this */
and text_node = any
and jsx_property = x =>
  M.identifier
  >>= (
    name =>
      M.assign
      >> (string_prop <|> M.braces(x))
      |= Reference(no_ctx(Variable(name)))
      ==> no_ctx
      % (value => (name, value))
  )
and string_prop = M.string ==> (s => StringLit(s))
and jsx_tags = input => M.parentheses(jsx_tag |> many, input)
and jsx_tag = input => (jsx_key_tag <|> jsx_class_prop_tag)(input)
and jsx_key_tag = M.identifier ==> (s => ElementKey(s))
and jsx_class_prop_tag = M.period >> M.identifier ==> (s => ElementClass(s));
