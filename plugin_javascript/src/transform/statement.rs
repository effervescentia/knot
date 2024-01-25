use crate::{
    javascript::{Expression, Statement},
    Options,
};
use kore::{invariant, str};
use lang::ast;

#[allow(clippy::multiple_inherent_impl)]
impl Statement {
    pub fn from_statement(
        value: &ast::shape::Statement,
        is_last: bool,
        opts: &Options,
    ) -> Vec<Self> {
        match &value.0 {
            ast::Statement::Expression(x) => {
                if is_last {
                    vec![Self::Return(Some(Expression::from_expression(x, opts)))]
                } else {
                    vec![Self::Expression(Expression::from_expression(x, opts))]
                }
            }

            ast::Statement::Variable(name, x) => {
                if is_last {
                    vec![
                        Self::Variable(name.clone(), Expression::from_expression(x, opts)),
                        Self::Return(None),
                    ]
                } else {
                    vec![Self::Variable(
                        name.clone(),
                        Expression::from_expression(x, opts),
                    )]
                }
            }
        }
    }

    pub fn from_declaration(
        path_to_root: &str,
        value: &ast::shape::Declaration,
        opts: &Options,
    ) -> Vec<Self> {
        fn parameter_name(suffix: &String) -> String {
            format!("$param_{suffix}")
        }

        match &value.0 {
            ast::Declaration::TypeAlias { .. } => vec![],

            ast::Declaration::Enumerated {
                storage: ast::Storage { binding, .. },
                variants,
            } => vec![Self::Variable(
                binding.clone(),
                Expression::Object(
                    variants
                        .iter()
                        .map(|(variant_name, variant_parameters)| {
                            let parameters = variant_parameters
                                .iter()
                                .enumerate()
                                .map(|(index, _)| parameter_name(&index.to_string()))
                                .collect::<Vec<_>>();

                            let results = [
                                vec![Expression::DotAccess(
                                    Box::new(Expression::Identifier(binding.clone())),
                                    variant_name.clone(),
                                )],
                                parameters
                                    .iter()
                                    .map(|x| Expression::Identifier(x.clone()))
                                    .collect(),
                            ]
                            .concat();

                            (
                                variant_name.clone(),
                                Expression::Function(
                                    Some(variant_name.clone()),
                                    parameters,
                                    vec![Self::Return(Some(Expression::Array(results)))],
                                ),
                            )
                        })
                        .collect(),
                ),
            )],

            ast::Declaration::Constant {
                storage: ast::Storage { binding, .. },
                value,
                ..
            } => vec![Self::Variable(
                binding.clone(),
                Expression::from_expression(value, opts),
            )],

            ast::Declaration::Function {
                storage: ast::Storage { binding, .. },
                parameters,
                body,
                ..
            }
            | ast::Declaration::View {
                storage: ast::Storage { binding, .. },
                parameters,
                body,
                ..
            } => {
                let statements = [
                    parameters
                        .iter()
                        .filter_map(|x| {
                            x.0.default_value.as_ref().map(|default| {
                                Self::Assignment(
                                    Expression::Identifier(x.0.binding.clone()),
                                    Expression::FunctionCall(
                                        Box::new(Expression::FunctionCall(
                                            Box::new(Expression::Identifier(str!(
                                                "$knot.plugin.get"
                                            ))),
                                            vec![
                                                Expression::String(str!("core")),
                                                Expression::String(str!("defaultParameter")),
                                                Expression::String(str!("1.0")),
                                            ],
                                        )),
                                        vec![
                                            Expression::Identifier(x.0.binding.clone()),
                                            Expression::from_expression(default, opts),
                                        ],
                                    ),
                                )
                            })
                        })
                        .collect::<Vec<_>>(),
                    match Expression::from_expression(body, opts) {
                        Expression::Closure(xs) => xs,

                        x => vec![Self::Return(Some(x))],
                    },
                ]
                .concat();

                vec![Self::Expression(Expression::Function(
                    Some(binding.clone()),
                    parameters.iter().map(|x| x.0.binding.clone()).collect(),
                    statements,
                ))]
            }

            ast::Declaration::Module {
                storage: ast::Storage { binding, .. },
                value,
            } => {
                let statements = [
                    Self::from_module(path_to_root, value, opts),
                    vec![Self::Return(Some(Expression::Object(
                        value
                            .0
                            .declarations
                            .iter()
                            .filter_map(|x| {
                                x.0.is_public().then(|| {
                                    (
                                        x.0.binding().clone(),
                                        Expression::Identifier(x.0.binding().clone()),
                                    )
                                })
                            })
                            .collect(),
                    )))],
                ]
                .concat();

                vec![Self::Variable(
                    binding.clone(),
                    Expression::Closure(statements),
                )]
            }
        }
    }

    // TODO: make this return a single value instead of an array
    pub fn from_import(
        path_to_root: &str,
        ast::shape::Import(ast::Import {
            source,
            path,
            alias,
        }): &ast::shape::Import,
        opts: &Options,
    ) -> Vec<Self> {
        let module_name = path.last().unwrap_or_else(|| {
            invariant!(
                "failed to get the implicit module name from the last section of the path {path:?}"
            )
        });
        let path = path.join("/");
        let import_path = match source {
            ast::ImportSource::Local => format!("./{path}.js"),
            ast::ImportSource::Root => format!("{path_to_root}/{path}.js"),
            ast::ImportSource::Named(name) => format!("{name}/{path}"),
            ast::ImportSource::Scoped { scope, name } => format!("@{scope}/{name}/{path}"),
        };

        vec![Self::module_import(
            &import_path,
            alias.as_ref().unwrap_or(module_name),
            opts,
        )]
    }

    pub fn from_module(
        path_to_root: &str,
        value: &ast::shape::Module,
        opts: &Options,
    ) -> Vec<Self> {
        let ast::Module {
            ref imports,
            ref declarations,
        } = value.0;

        let import_iter = imports
            .iter()
            .flat_map(|x| Self::from_import(path_to_root, x, opts));

        let declaration_iter = declarations
            .iter()
            .flat_map(|x| Self::from_declaration(path_to_root, x, opts));

        import_iter.chain(declaration_iter).collect()
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        javascript::{Expression, Statement},
        Mode, Module, Options,
    };
    use kore::str;
    use lang::ast;

    const OPTIONS: Options = Options {
        mode: Mode::Prod,
        module: Module::ESM,
    };

    mod statement {
        use super::*;

        #[test]
        fn expression() {
            assert_eq!(
                Statement::from_statement(
                    &ast::shape::Statement(ast::Statement::Expression(ast::shape::Expression(
                        ast::Expression::Primitive(ast::Primitive::Nil)
                    ))),
                    false,
                    &OPTIONS
                ),
                vec![Statement::Expression(Expression::Null)]
            );
        }

        #[test]
        fn last_expression() {
            assert_eq!(
                Statement::from_statement(
                    &ast::shape::Statement(ast::Statement::Expression(ast::shape::Expression(
                        ast::Expression::Primitive(ast::Primitive::Nil)
                    ))),
                    true,
                    &OPTIONS
                ),
                vec![Statement::Return(Some(Expression::Null))]
            );
        }

        #[test]
        fn variable() {
            assert_eq!(
                Statement::from_statement(
                    &ast::shape::Statement(ast::Statement::Variable(
                        str!("foo"),
                        ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil))
                    )),
                    false,
                    &OPTIONS
                ),
                vec![Statement::Variable(str!("foo"), Expression::Null)]
            );
        }

        #[test]
        fn last_variable() {
            assert_eq!(
                Statement::from_statement(
                    &ast::shape::Statement(ast::Statement::Variable(
                        str!("foo"),
                        ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil))
                    )),
                    true,
                    &OPTIONS
                ),
                vec![
                    Statement::Variable(str!("foo"), Expression::Null),
                    Statement::Return(None)
                ]
            );
        }
    }

    mod declaration {
        use super::*;
        use lang::ast;

        #[test]
        fn type_alias() {
            assert_eq!(
                Statement::from_declaration(
                    ".",
                    &ast::shape::Declaration(ast::Declaration::TypeAlias {
                        storage: ast::Storage::public(str!("foo")),
                        value: ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                            ast::TypePrimitive::Nil
                        ))
                    }),
                    &OPTIONS
                ),
                vec![]
            );
        }

        #[test]
        fn enumerated() {
            assert_eq!(
                Statement::from_declaration(
                    ".",
                    &ast::shape::Declaration(ast::Declaration::Enumerated {
                        storage: ast::Storage::public(str!("foo")),
                        variants: vec![
                            (
                                str!("Bar"),
                                vec![ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                                    ast::TypePrimitive::Nil
                                )),]
                            ),
                            (str!("Fizz"), vec![])
                        ]
                    }),
                    &OPTIONS
                ),
                vec![Statement::Variable(
                    str!("foo"),
                    Expression::Object(vec![
                        (
                            str!("Bar"),
                            Expression::Function(
                                Some(str!("Bar")),
                                vec![str!("$param_0")],
                                vec![Statement::Return(Some(Expression::Array(vec![
                                    Expression::DotAccess(
                                        Box::new(Expression::Identifier(str!("foo"))),
                                        str!("Bar")
                                    ),
                                    Expression::Identifier(str!("$param_0"))
                                ])))]
                            )
                        ),
                        (
                            str!("Fizz"),
                            Expression::Function(
                                Some(str!("Fizz")),
                                vec![],
                                vec![Statement::Return(Some(Expression::Array(vec![
                                    Expression::DotAccess(
                                        Box::new(Expression::Identifier(str!("foo"))),
                                        str!("Fizz")
                                    ),
                                ])))]
                            )
                        )
                    ])
                )]
            );
        }

        #[test]
        fn constant() {
            assert_eq!(
                Statement::from_declaration(
                    ".",
                    &ast::shape::Declaration(ast::Declaration::Constant {
                        storage: ast::Storage::public(str!("foo")),
                        value_type: None,
                        value: ast::shape::Expression(ast::Expression::Primitive(
                            ast::Primitive::Nil
                        ))
                    }),
                    &OPTIONS
                ),
                vec![Statement::Variable(str!("foo"), Expression::Null)]
            );
        }

        #[test]
        fn function() {
            assert_eq!(
                Statement::from_declaration(
                    ".",
                    &ast::shape::Declaration(ast::Declaration::Function {
                        storage: ast::Storage::public(str!("foo")),
                        parameters: vec![
                            ast::shape::Parameter(ast::Parameter::new(str!("bar"), None, None)),
                            ast::shape::Parameter(ast::Parameter::new(
                                str!("fizz"),
                                None,
                                Some(ast::shape::Expression(ast::Expression::Primitive(
                                    ast::Primitive::Boolean(true)
                                )))
                            )),
                        ],
                        body_type: None,
                        body: ast::shape::Expression(ast::Expression::Primitive(
                            ast::Primitive::Nil
                        ))
                    }),
                    &OPTIONS
                ),
                vec![Statement::Expression(Expression::Function(
                    Some(str!("foo")),
                    vec![str!("bar"), str!("fizz")],
                    vec![
                        Statement::Assignment(
                            Expression::Identifier(str!("fizz")),
                            Expression::FunctionCall(
                                Box::new(Expression::FunctionCall(
                                    Box::new(Expression::Identifier(str!("$knot.plugin.get"))),
                                    vec![
                                        Expression::String(str!("core")),
                                        Expression::String(str!("defaultParameter")),
                                        Expression::String(str!("1.0"))
                                    ]
                                )),
                                vec![
                                    Expression::Identifier(str!("fizz")),
                                    Expression::Boolean(true)
                                ]
                            )
                        ),
                        Statement::Return(Some(Expression::Null))
                    ]
                ))]
            );
        }

        #[test]
        fn function_closure_body() {
            assert_eq!(
                Statement::from_declaration(
                    ".",
                    &ast::shape::Declaration(ast::Declaration::Function {
                        storage: ast::Storage::public(str!("foo")),
                        parameters: vec![],
                        body_type: None,
                        body: ast::shape::Expression(ast::Expression::Closure(vec![
                            ast::shape::Statement(ast::Statement::Variable(
                                str!("bar"),
                                ast::shape::Expression(ast::Expression::Primitive(
                                    ast::Primitive::Nil
                                ))
                            )),
                            ast::shape::Statement(ast::Statement::Expression(
                                ast::shape::Expression(ast::Expression::Identifier(str!("bar")))
                            ))
                        ]))
                    }),
                    &OPTIONS
                ),
                vec![Statement::Expression(Expression::Function(
                    Some(str!("foo")),
                    vec![],
                    vec![
                        Statement::Variable(str!("bar"), Expression::Null),
                        Statement::Return(Some(Expression::Identifier(str!("bar"))))
                    ]
                ))]
            );
        }

        #[test]
        fn view() {
            assert_eq!(
                Statement::from_declaration(
                    ".",
                    &ast::shape::Declaration(ast::Declaration::View {
                        storage: ast::Storage::public(str!("foo")),
                        parameters: vec![
                            ast::shape::Parameter(ast::Parameter::new(str!("bar"), None, None)),
                            ast::shape::Parameter(ast::Parameter::new(
                                str!("fizz"),
                                None,
                                Some(ast::shape::Expression(ast::Expression::Primitive(
                                    ast::Primitive::Boolean(true)
                                )))
                            )),
                        ],
                        body: ast::shape::Expression(ast::Expression::Primitive(
                            ast::Primitive::Nil
                        ))
                    }),
                    &OPTIONS
                ),
                vec![Statement::Expression(Expression::Function(
                    Some(str!("foo")),
                    vec![str!("bar"), str!("fizz")],
                    vec![
                        Statement::Assignment(
                            Expression::Identifier(str!("fizz")),
                            Expression::FunctionCall(
                                Box::new(Expression::FunctionCall(
                                    Box::new(Expression::Identifier(str!("$knot.plugin.get"))),
                                    vec![
                                        Expression::String(str!("core")),
                                        Expression::String(str!("defaultParameter")),
                                        Expression::String(str!("1.0"))
                                    ]
                                )),
                                vec![
                                    Expression::Identifier(str!("fizz")),
                                    Expression::Boolean(true)
                                ]
                            )
                        ),
                        Statement::Return(Some(Expression::Null))
                    ]
                ))]
            );
        }

        #[test]
        fn module() {
            assert_eq!(
                Statement::from_declaration(
                    ".",
                    &ast::shape::Declaration(ast::Declaration::Module {
                        storage: ast::Storage::public(str!("foo")),
                        value: ast::shape::Module(ast::Module {
                            imports: vec![],
                            declarations: vec![
                                ast::shape::Declaration(ast::Declaration::Constant {
                                    storage: ast::Storage::public(str!("bar")),
                                    value_type: None,
                                    value: ast::shape::Expression(ast::Expression::Primitive(
                                        ast::Primitive::Nil
                                    ))
                                }),
                                ast::shape::Declaration(ast::Declaration::Constant {
                                    storage: ast::Storage::public(str!("fizz")),
                                    value_type: None,
                                    value: ast::shape::Expression(ast::Expression::Primitive(
                                        ast::Primitive::Nil
                                    ))
                                })
                            ]
                        })
                    }),
                    &OPTIONS
                ),
                vec![Statement::Variable(
                    str!("foo"),
                    Expression::Closure(vec![
                        Statement::Variable(str!("bar"), Expression::Null),
                        Statement::Variable(str!("fizz"), Expression::Null),
                        Statement::Return(Some(Expression::Object(vec![(
                            str!("bar"),
                            Expression::Identifier(str!("bar"))
                        )]))),
                    ])
                )]
            );
        }
    }
}
