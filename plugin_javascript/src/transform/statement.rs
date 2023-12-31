use crate::{
    javascript::{Expression, Statement},
    Options,
};
use kore::{invariant, str};
use lang::ast::{self, storage::Storage};

#[allow(clippy::multiple_inherent_impl)]
impl Statement {
    pub fn from_statement(value: &ast::StatementShape, is_last: bool, opts: &Options) -> Vec<Self> {
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

    pub fn from_declaration(value: &ast::DeclarationShape, opts: &Options) -> Vec<Self> {
        fn parameter_name(suffix: &String) -> String {
            format!("$param_{suffix}")
        }

        match &value.0 {
            ast::Declaration::TypeAlias { .. } => vec![],

            ast::Declaration::Enumerated {
                name: Storage(_, name),
                variants,
            } => vec![Self::Variable(
                name.clone(),
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
                                    Box::new(Expression::Identifier(name.clone())),
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
                name: Storage(_, name),
                value,
                ..
            } => vec![Self::Variable(
                name.clone(),
                Expression::from_expression(value, opts),
            )],

            ast::Declaration::Function {
                name: Storage(_, name),
                parameters,
                body,
                ..
            }
            | ast::Declaration::View {
                name: Storage(_, name),
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
                                    Expression::Identifier(x.0.name.clone()),
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
                                            Expression::Identifier(x.0.name.clone()),
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
                    Some(name.clone()),
                    parameters.iter().map(|x| x.0.name.clone()).collect(),
                    statements,
                ))]
            }

            ast::Declaration::Module {
                name: Storage(_, name),
                value,
            } => {
                let statements = [
                    Self::from_module(value, opts),
                    vec![Self::Return(Some(Expression::Object(
                        value
                            .0
                            .declarations
                            .iter()
                            .filter_map(|x| {
                                x.0.is_public().then(|| {
                                    (
                                        x.0.name().clone(),
                                        Expression::Identifier(x.0.name().clone()),
                                    )
                                })
                            })
                            .collect(),
                    )))],
                ]
                .concat();

                vec![Self::Variable(
                    name.clone(),
                    Expression::Closure(statements),
                )]
            }
        }
    }

    pub fn from_import(
        ast::ImportShape(ast::Import {
            source,
            path,
            aliases,
        }): &ast::ImportShape,
        opts: &Options,
    ) -> Vec<Self> {
        let base = match source {
            ast::ImportSource::Local => str!("."),
            ast::ImportSource::Root => str!("@"),
            ast::ImportSource::Named(name) => name.clone(),
            ast::ImportSource::Scoped { scope, name } => format!("@{scope}/{name}"),
        };

        let namespace = [vec![base], path.clone()].concat().join("/");
        let module_name = path.last().unwrap_or_else(|| {
            invariant!(
                "failed to get the implicit module name from the last section of the path {path:?}"
            )
        });

        match aliases {
            Some(xs) => {
                let (mut imports, named) = xs.iter().fold(
                    (vec![], vec![]),
                    |(mut module_imports, mut named_imports), (target, alias)| {
                        match target {
                            ast::ImportTarget::Module => {
                                module_imports.push(Self::module_import(
                                    &namespace,
                                    alias.as_ref().unwrap_or(module_name),
                                    opts,
                                ));
                            }

                            ast::ImportTarget::Named(name) => {
                                named_imports.push((name.clone(), alias.clone()));
                            }
                        }

                        (module_imports, named_imports)
                    },
                );

                if !named.is_empty() {
                    imports.extend(Self::import(&namespace, named, opts));
                }

                imports
            }

            None => {
                vec![Self::module_import(&namespace, module_name, opts)]
            }
        }
    }

    pub fn from_module(value: &ast::ModuleShape, opts: &Options) -> Vec<Self> {
        let ast::Module {
            ref imports,
            ref declarations,
        } = value.0;

        let import_iter = imports.iter().flat_map(|x| Self::from_import(x, opts));

        let declaration_iter = declarations
            .iter()
            .flat_map(|x| Self::from_declaration(x, opts));

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
                    &ast::StatementShape(ast::Statement::Expression(ast::ExpressionShape(
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
                    &ast::StatementShape(ast::Statement::Expression(ast::ExpressionShape(
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
                    &ast::StatementShape(ast::Statement::Variable(
                        str!("foo"),
                        ast::ExpressionShape(ast::Expression::Primitive(ast::Primitive::Nil))
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
                    &ast::StatementShape(ast::Statement::Variable(
                        str!("foo"),
                        ast::ExpressionShape(ast::Expression::Primitive(ast::Primitive::Nil))
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
        use lang::ast::{
            storage::{Storage, Visibility},
            Module, Parameter,
        };

        #[test]
        fn type_alias() {
            assert_eq!(
                Statement::from_declaration(
                    &ast::DeclarationShape(ast::Declaration::TypeAlias {
                        name: Storage(Visibility::Public, str!("foo")),
                        value: ast::TypeExpressionShape(ast::TypeExpression::Nil)
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
                    &ast::DeclarationShape(ast::Declaration::Enumerated {
                        name: Storage(Visibility::Public, str!("foo")),
                        variants: vec![
                            (
                                str!("Bar"),
                                vec![ast::TypeExpressionShape(ast::TypeExpression::Nil),]
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
                    &ast::DeclarationShape(ast::Declaration::Constant {
                        name: Storage(Visibility::Public, str!("foo")),
                        value_type: None,
                        value: ast::ExpressionShape(ast::Expression::Primitive(
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
                    &ast::DeclarationShape(ast::Declaration::Function {
                        name: Storage(Visibility::Public, str!("foo")),
                        parameters: vec![
                            ast::ParameterShape(Parameter::new(str!("bar"), None, None)),
                            ast::ParameterShape(Parameter::new(
                                str!("fizz"),
                                None,
                                Some(ast::ExpressionShape(ast::Expression::Primitive(
                                    ast::Primitive::Boolean(true)
                                )))
                            )),
                        ],
                        body_type: None,
                        body: ast::ExpressionShape(ast::Expression::Primitive(ast::Primitive::Nil))
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
                    &ast::DeclarationShape(ast::Declaration::Function {
                        name: Storage(Visibility::Public, str!("foo")),
                        parameters: vec![],
                        body_type: None,
                        body: ast::ExpressionShape(ast::Expression::Closure(vec![
                            ast::StatementShape(ast::Statement::Variable(
                                str!("bar"),
                                ast::ExpressionShape(ast::Expression::Primitive(
                                    ast::Primitive::Nil
                                ))
                            )),
                            ast::StatementShape(ast::Statement::Expression(ast::ExpressionShape(
                                ast::Expression::Identifier(str!("bar"))
                            )))
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
                    &ast::DeclarationShape(ast::Declaration::View {
                        name: Storage(Visibility::Public, str!("foo")),
                        parameters: vec![
                            ast::ParameterShape(Parameter::new(str!("bar"), None, None)),
                            ast::ParameterShape(Parameter::new(
                                str!("fizz"),
                                None,
                                Some(ast::ExpressionShape(ast::Expression::Primitive(
                                    ast::Primitive::Boolean(true)
                                )))
                            )),
                        ],
                        body: ast::ExpressionShape(ast::Expression::Primitive(ast::Primitive::Nil))
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
                    &ast::DeclarationShape(ast::Declaration::Module {
                        name: Storage(Visibility::Public, str!("foo")),
                        value: ast::ModuleShape(Module {
                            imports: vec![],
                            declarations: vec![
                                ast::DeclarationShape(ast::Declaration::Constant {
                                    name: Storage(Visibility::Public, str!("bar")),
                                    value_type: None,
                                    value: ast::ExpressionShape(ast::Expression::Primitive(
                                        ast::Primitive::Nil
                                    ))
                                }),
                                ast::DeclarationShape(ast::Declaration::Constant {
                                    name: Storage(Visibility::Private, str!("fizz")),
                                    value_type: None,
                                    value: ast::ExpressionShape(ast::Expression::Primitive(
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
