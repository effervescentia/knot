open Kore;

module Token = {
  type t =
    | Skip
    | Join
    | Identifier(string)
    | Primitive(AST.Result.primitive_t);
};

type t = RangeTree.t(Token.t);

let __skip: t = BinaryTree.create((Range.zero, Token.Skip));

let _join = (left: t, right: t) =>
  switch (left, right) {
  | ({value: (_, Skip), _}, {value: (_, Skip), _}) => __skip
  | (only, {value: (_, Skip), _})
  | ({value: (_, Skip), _}, only) => only
  | _ =>
    BinaryTree.create(
      ~left,
      ~right,
      ((left.value |> fst |> fst, right.value |> fst |> snd), Join),
    )
  };

let _fold = f => List.fold_left((acc, x) => _join(acc, f(x)), __skip);

let _wrap = (range, tree: t) =>
  tree.value |> fst == range
    ? tree : BinaryTree.create(~left=tree, (range, Join));

/* static */

let of_untyped_id = ((id, (_, range))) =>
  BinaryTree.create((range, Token.Identifier(id)));

let rec of_list = (xs: list(t)): t =>
  switch (xs) {
  | [] => __skip
  | [x] => x
  | _ =>
    switch (xs |> List.divide |> Tuple.map2(of_list)) {
    | ({value: (_, Skip), _}, {value: (_, Skip), _}) => __skip
    | ({value: (_, Skip), _}, only)
    | (only, {value: (_, Skip), _}) => only
    | (head, tail) => _join(head, tail)
    }
  };

let rec of_effect =
  AST.Expression.(
    fun
    | (Primitive(prim), (_, range)) =>
      BinaryTree.create((range, Token.Primitive(prim)))
    | (Identifier(id), (_, range)) => Node.raw(id, range) |> of_untyped_id
    | (KSX(ksx), (_, range)) => ksx |> of_ksx |> _wrap(range)
    | (Group(expr), _) => expr |> of_effect |> _wrap(Node.get_range(expr))
    | (BinaryOp(_, lhs, rhs), _) =>
      _join(
        lhs |> of_effect |> _wrap(Node.get_range(lhs)),
        rhs |> of_effect |> _wrap(Node.get_range(rhs)),
      )
    | (UnaryOp(_, expr), _) =>
      expr |> of_effect |> _wrap(Node.get_range(expr))
    | (Closure(stmts), _) => stmts |> List.map(fst % of_stmt) |> of_list
    | (DotAccess(expr, _), _) =>
      expr |> of_effect |> _wrap(Node.get_range(expr))
    | (BindStyle(_, view, style), _) =>
      _join(
        view |> of_effect |> _wrap(Node.get_range(view)),
        style |> of_effect |> _wrap(Node.get_range(style)),
      )
    | (FunctionCall(expr, args), _) =>
      _join(
        expr |> of_effect |> _wrap(Node.get_range(expr)),
        args |> List.map(of_effect) |> of_list,
      )
    | (Style(rules), _) =>
      rules |> List.map(fst % snd % of_effect) |> of_list
  )

and of_ksx =
  AST.Expression.(
    fun
    | Fragment(children) =>
      children
      |> List.map(child =>
           child |> of_jsx_child |> _wrap(Node.get_range(child))
         )
      |> of_list

    | Tag(_, (id, (_, range)), styles, attrs, children) =>
      [Node.raw(id, range) |> of_untyped_id]
      @ (styles |> List.map(of_effect))
      @ (
        attrs
        |> List.map(attr =>
             attr |> fst |> of_jsx_attr |> _wrap(Node.get_range(attr))
           )
      )
      @ (
        children
        |> List.map(child =>
             child |> of_jsx_child |> _wrap(Node.get_range(child))
           )
      )
      |> of_list
  )

and of_jsx_child =
  AST.Expression.(
    fun
    | (Node(tag), (_, range)) => tag |> of_ksx |> _wrap(range)
    | (InlineExpression(expr), _) =>
      expr |> of_effect |> _wrap(Node.get_range(expr))
    | (Text(text), (_, range)) =>
      BinaryTree.create((range, Token.Primitive(String(text))))
  )

and of_jsx_attr =
  AST.Expression.(
    fun
    | (id, None) => of_untyped_id(id)
    | (id, Some(expr)) =>
      _join(
        of_untyped_id(id),
        expr |> of_effect |> _wrap(Node.get_range(expr)),
      )
  )

and of_stmt =
  AST.Expression.(
    fun
    | Variable(name, expr) =>
      _join(
        of_untyped_id(name),
        expr |> of_effect |> _wrap(Node.get_range(expr)),
      )
    | Effect(expr) => expr |> of_effect |> _wrap(Node.get_range(expr))
  );

let of_args = args =>
  args
  |> _fold(
       fst
       % (
         ((name, _, default)) => {
           ...of_untyped_id(name),
           right: default |?> of_effect,
         }
       ),
     );

let of_export =
  AST.Module.(
    fun
    | Constant(expr) => expr |> of_effect |> _wrap(Node.get_range(expr))
    | Enumerated(variants) =>
      variants
      |> List.map(((name, _)) => [of_untyped_id(name)])
      |> List.flatten
      |> of_list
    | Function(args, expr) =>
      _join(
        of_args(args),
        expr |> of_effect |> _wrap(Node.get_range(expr)),
      )
    | View(props, _, expr) =>
      _join(
        of_args(props),
        expr |> of_effect |> _wrap(Node.get_range(expr)),
      )
  );

let of_named_import =
  AST.Module.(
    fun
    | (id, None) => of_untyped_id(id)
    | (id, Some(alias)) =>
      (id, alias) |> Tuple.map2(of_untyped_id) |> Tuple.join2(_join)
  );

let of_mod_stmt =
  AST.Module.(
    fun
    | StdlibImport(imports) =>
      imports
      |> _fold(
           fst
           % (
             fun
             | (id, None) => of_untyped_id(id)
             | (id, Some(alias)) =>
               (id, alias)
               |> Tuple.map2(of_untyped_id)
               |> Tuple.join2(_join)
           ),
         )
    | Import(_, main_import, named_imports) =>
      (
        main_import
        |> Option.map(main_import' => [of_untyped_id(main_import')])
        |?: []
      )
      @ (named_imports |> List.map(fst % of_named_import))
      |> of_list
    | Export(_, id, (decl, _)) =>
      _join(of_untyped_id(id), of_export(decl))
  );

let of_ast = (program: AST.Module.program_t) =>
  program |> _fold(fst % of_mod_stmt);

/* pretty printing */

let pp: Fmt.t(t) =
  (ppf, tree: t) =>
    BinaryTree.pp(
      (ppf, ((start, end_), token)) =>
        Fmt.pf(
          ppf,
          "%s %s",
          Token.(
            switch (token) {
            | Join => ""
            | Skip => "[skip]"
            | Identifier(id) => id
            | Primitive(prim) => prim |> ~@KPrimitive.Plugin.pp
            }
          ),
          Range.create(start, end_) |> ~@Range.pp,
        ),
      ppf,
      tree,
    );
