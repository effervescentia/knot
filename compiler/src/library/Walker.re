open Infix;

type target_t =
  | ModuleStatement(AST.module_statement_t)
  | Import(AST.import_t)
  | Declaration(AST.declaration_t)
  | Statement(AST.statement_t)
  | Expression(AST.expression_t)
  | Identifier(string)
  | JSX(AST.jsx_t)
  | JSXChild(AST.jsx_child_t)
  | JSXAttribute(AST.jsx_attribute_t)
  | Primitive(AST.primitive_t);

let rec iter = (f: target_t => unit, prog: AST.program_t) =>
  prog
  |> List.iter(x => {
       f(ModuleStatement(x));
       iter_mod_stmt(f, x);
     })

and iter_mod_stmt = f =>
  fst
  % AST.(
      fun
      | Declaration(_, decl) => {
          f(Declaration(decl));
          iter_decl(f, decl);
        }
      | Import(_, imports) => imports |> List.iter(x => f(Import(x)))
      | StandardImport(imports) =>
        imports
        |> List.iter(Node.map(AST.of_named_import) % (x => f(Import(x))))
    )

and iter_decl = f =>
  fst
  % AST.(
      fun
      | Constant(expr) => _bind_expr(f, expr)
      | Enumerated(_) => ()
      | Function(args, expr) =>
        (args |> List.filter_map(((arg, _)) => arg.default))
        @ [expr]
        |> List.iter(_bind_expr(f))
      | View(props, _, expr) =>
        (props |> List.filter_map(((arg, _)) => arg.default))
        @ [expr]
        |> List.iter(_bind_expr(f))
    )

and iter_stmt = f =>
  fst
  % (
    fun
    | AST.Expression(expr) => _bind_expr(f, expr)
    | AST.Variable(_, expr) => _bind_expr(f, expr)
  )

and _bind_expr = (f, x) => {
  f(Expression(x));
  iter_expr(f, x);
}
and iter_expr = f => {
  fst
  % AST.(
      fun
      | Primitive(x) => f(Primitive(x))
      | Identifier(x) => f(Identifier(x))
      | JSX(x) => _bind_jsx(f, x)
      | Group(x)
      | UnaryOp(_, x) => _bind_expr(f, x)
      | BinaryOp(_, l, r) => [l, r] |> List.iter(_bind_expr(f))
      | Closure(stmts) =>
        stmts
        |> List.iter(x => {
             f(Statement(x));
             iter_stmt(f, x);
           })
      | DotAccess(expr, _) => _bind_expr(f, expr)
      | FunctionCall(expr, args) => {
          _bind_expr(f, expr);
          args |> List.iter(_bind_expr(f));
        }
      | Style(rules) =>
        rules |> List.map(fst % snd) |> List.iter(_bind_expr(f))
    );
}

and _bind_jsx = (f, x) => {
  f(JSX(x));
  iter_jsx(f, x);
}
and iter_jsx = f =>
  AST.(
    fun
    | Fragment(children) => children |> List.iter(_bind_jsx_child(f))
    | Tag(_, attrs, children) => {
        attrs
        |> List.iter(x => {
             f(JSXAttribute(x));
             iter_jsx_attr(f, x);
           });
        children |> List.iter(_bind_jsx_child(f));
      }
    | Component((id, _), attrs, children) => {
        f(Identifier(id));
        attrs
        |> List.iter(x => {
             f(JSXAttribute(x));
             iter_jsx_attr(f, x);
           });
        children |> List.iter(_bind_jsx_child(f));
      }
  )

and _bind_jsx_child = (f, x) => {
  f(JSXChild(x));
  iter_jsx_child(f, x);
}
and iter_jsx_child = f =>
  fst
  % AST.(
      fun
      | Node(x) => _bind_jsx(f, x)
      | InlineExpression(x) => _bind_expr(f, x)
      | Text(_) => ()
    )

and iter_jsx_attr = f =>
  fst
  % AST.(
      fun
      | Property(_, Some(x))
      | Class(_, Some(x)) => _bind_expr(f, x)
      | _ => ()
    );
