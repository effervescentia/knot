open Core;
open Context;

/* </abc> */
let close_tag = name =>
  M.jsx_open_end
  >> push(JSXEndTag)
  >> M.exact_identifier(name)
  >> M.r_chev
  << pop;

let rec expr = x => fragment(x) <|> element(x)
/* should figure out why `fragment` cannot appear in `nested_expr` */
and nested_expr = x => eval_node(x) <|> element(x)
and nested_exprs = x => nested_expr(x) |> many
and element = x =>
  M.l_chev
  >> push(JSXStartTag)
  >> M.identifier
  << pop
  >>= (
    name =>
      jsx_property(x)
      |> many
      >>= (
        props =>
          container_element(x, name)
          <|> self_closing_element
          ==> (children => Element(name, props, children))
      )
  )
and container_element = (x, name) =>
  M.r_chev
  /* >> swap(JSXContent)  */
  >> nested_exprs(x)
  << close_tag(name)
and self_closing_element =
  M.jsx_self_close
  /* >> pop  */
  >> return([])
and fragment = x =>
  M.jsx_start_frag
  >> nested_exprs(x)
  << M.jsx_end_frag
  ==> (children => Fragment(children))
and eval_node = x => M.braces(x) ==> (ex => EvalNode(ex))
/* M.braces(push(Normal) >> x) << pop ==> (ex => EvalNode(ex)) */
/* figure out how to do this */
and text_node = any
and jsx_property = x =>
  M.identifier
  >>= (
    name =>
      M.assign
      >> (string_prop <|> M.braces(x))
      |= Reference(Variable(name))
      ==> (value => (name, value))
  )
and string_prop = M.string ==> (s => StringLit(s));
