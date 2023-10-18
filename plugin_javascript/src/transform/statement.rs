use crate::{
    javascript::{Expression, Statement},
    Options,
};
use lang::ast::{self, storage::Storage, DeclarationShape, ModuleShape, StatementShape};

impl Statement {
    pub fn from_statement(value: &StatementShape, is_last: bool, opts: &Options) -> Vec<Self> {
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

    pub fn from_declaration(value: &DeclarationShape, opts: &Options) -> Vec<Self> {
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

                            let results = vec![
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
                let statements = vec![
                    parameters
                        .iter()
                        .filter_map(|x| {
                            if let Some(default) = &x.0.default_value {
                                Some(Statement::Assignment(
                                    Expression::Identifier(x.0.name.clone()),
                                    Expression::FunctionCall(
                                        Box::new(Expression::FunctionCall(
                                            Box::new(Expression::Identifier(String::from(
                                                "$knot.plugin.get",
                                            ))),
                                            vec![
                                                Expression::String(String::from("core")),
                                                Expression::String(String::from(
                                                    "defaultParameter",
                                                )),
                                                Expression::String(String::from("1.0")),
                                            ],
                                        )),
                                        vec![
                                            Expression::Identifier(x.0.name.clone()),
                                            Expression::from_expression(default, opts),
                                        ],
                                    ),
                                ))
                            } else {
                                None
                            }
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
                let statements = vec![
                    Self::from_module(value, opts),
                    vec![Self::Return(Some(Expression::Object(
                        value
                            .0
                            .declarations
                            .iter()
                            .filter_map(|x| {
                                if x.0.is_public() {
                                    Some((
                                        x.0.name().clone(),
                                        Expression::Identifier(x.0.name().clone()),
                                    ))
                                } else {
                                    None
                                }
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

    pub fn from_module(value: &ModuleShape, opts: &Options) -> Vec<Self> {
        value
            .0
            .declarations
            .iter()
            .flat_map(|x| Self::from_declaration(x, opts))
            .collect()
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        javascript::{Expression, Statement},
        Mode, Module, Options,
    };
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
                        String::from("foo"),
                        ast::ExpressionShape(ast::Expression::Primitive(ast::Primitive::Nil))
                    )),
                    false,
                    &OPTIONS
                ),
                vec![Statement::Variable(String::from("foo"), Expression::Null)]
            );
        }

        #[test]
        fn last_variable() {
            assert_eq!(
                Statement::from_statement(
                    &ast::StatementShape(ast::Statement::Variable(
                        String::from("foo"),
                        ast::ExpressionShape(ast::Expression::Primitive(ast::Primitive::Nil))
                    )),
                    true,
                    &OPTIONS
                ),
                vec![
                    Statement::Variable(String::from("foo"), Expression::Null),
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
                        name: Storage(Visibility::Public, String::from("foo")),
                        value: ast::TypeExpressionShape(ast::TypeExpression::Nil)
                    }),
                    &OPTIONS
                ),
                vec![]
            )
        }

        #[test]
        fn enumerated() {
            assert_eq!(
                Statement::from_declaration(
                    &ast::DeclarationShape(ast::Declaration::Enumerated {
                        name: Storage(Visibility::Public, String::from("foo")),
                        variants: vec![
                            (
                                String::from("Bar"),
                                vec![ast::TypeExpressionShape(ast::TypeExpression::Nil),]
                            ),
                            (String::from("Fizz"), vec![])
                        ]
                    }),
                    &OPTIONS
                ),
                vec![Statement::Variable(
                    String::from("foo"),
                    Expression::Object(vec![
                        (
                            String::from("Bar"),
                            Expression::Function(
                                Some(String::from("Bar")),
                                vec![String::from("$param_0")],
                                vec![Statement::Return(Some(Expression::Array(vec![
                                    Expression::DotAccess(
                                        Box::new(Expression::Identifier(String::from("foo"))),
                                        String::from("Bar")
                                    ),
                                    Expression::Identifier(String::from("$param_0"))
                                ])))]
                            )
                        ),
                        (
                            String::from("Fizz"),
                            Expression::Function(
                                Some(String::from("Fizz")),
                                vec![],
                                vec![Statement::Return(Some(Expression::Array(vec![
                                    Expression::DotAccess(
                                        Box::new(Expression::Identifier(String::from("foo"))),
                                        String::from("Fizz")
                                    ),
                                ])))]
                            )
                        )
                    ])
                )]
            )
        }

        #[test]
        fn constant() {
            assert_eq!(
                Statement::from_declaration(
                    &ast::DeclarationShape(ast::Declaration::Constant {
                        name: Storage(Visibility::Public, String::from("foo")),
                        value_type: None,
                        value: ast::ExpressionShape(ast::Expression::Primitive(
                            ast::Primitive::Nil
                        ))
                    }),
                    &OPTIONS
                ),
                vec![Statement::Variable(String::from("foo"), Expression::Null)]
            )
        }

        #[test]
        fn function() {
            assert_eq!(
                Statement::from_declaration(
                    &ast::DeclarationShape(ast::Declaration::Function {
                        name: Storage(Visibility::Public, String::from("foo")),
                        parameters: vec![
                            ast::ParameterShape(Parameter::new(String::from("bar"), None, None)),
                            ast::ParameterShape(Parameter::new(
                                String::from("fizz"),
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
                    Some(String::from("foo")),
                    vec![String::from("bar"), String::from("fizz")],
                    vec![
                        Statement::Assignment(
                            Expression::Identifier(String::from("fizz")),
                            Expression::FunctionCall(
                                Box::new(Expression::FunctionCall(
                                    Box::new(Expression::Identifier(String::from(
                                        "$knot.plugin.get"
                                    ))),
                                    vec![
                                        Expression::String(String::from("core")),
                                        Expression::String(String::from("defaultParameter")),
                                        Expression::String(String::from("1.0"))
                                    ]
                                )),
                                vec![
                                    Expression::Identifier(String::from("fizz")),
                                    Expression::Boolean(true)
                                ]
                            )
                        ),
                        Statement::Return(Some(Expression::Null))
                    ]
                ))]
            )
        }

        #[test]
        fn function_closure_body() {
            assert_eq!(
                Statement::from_declaration(
                    &ast::DeclarationShape(ast::Declaration::Function {
                        name: Storage(Visibility::Public, String::from("foo")),
                        parameters: vec![],
                        body_type: None,
                        body: ast::ExpressionShape(ast::Expression::Closure(vec![
                            ast::StatementShape(ast::Statement::Variable(
                                String::from("bar"),
                                ast::ExpressionShape(ast::Expression::Primitive(
                                    ast::Primitive::Nil
                                ))
                            )),
                            ast::StatementShape(ast::Statement::Expression(ast::ExpressionShape(
                                ast::Expression::Identifier(String::from("bar"))
                            )))
                        ]))
                    }),
                    &OPTIONS
                ),
                vec![Statement::Expression(Expression::Function(
                    Some(String::from("foo")),
                    vec![],
                    vec![
                        Statement::Variable(String::from("bar"), Expression::Null),
                        Statement::Return(Some(Expression::Identifier(String::from("bar"))))
                    ]
                ))]
            )
        }

        #[test]
        fn view() {
            assert_eq!(
                Statement::from_declaration(
                    &ast::DeclarationShape(ast::Declaration::View {
                        name: Storage(Visibility::Public, String::from("foo")),
                        parameters: vec![
                            ast::ParameterShape(Parameter::new(String::from("bar"), None, None)),
                            ast::ParameterShape(Parameter::new(
                                String::from("fizz"),
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
                    Some(String::from("foo")),
                    vec![String::from("bar"), String::from("fizz")],
                    vec![
                        Statement::Assignment(
                            Expression::Identifier(String::from("fizz")),
                            Expression::FunctionCall(
                                Box::new(Expression::FunctionCall(
                                    Box::new(Expression::Identifier(String::from(
                                        "$knot.plugin.get"
                                    ))),
                                    vec![
                                        Expression::String(String::from("core")),
                                        Expression::String(String::from("defaultParameter")),
                                        Expression::String(String::from("1.0"))
                                    ]
                                )),
                                vec![
                                    Expression::Identifier(String::from("fizz")),
                                    Expression::Boolean(true)
                                ]
                            )
                        ),
                        Statement::Return(Some(Expression::Null))
                    ]
                ))]
            )
        }

        #[test]
        fn module() {
            assert_eq!(
                Statement::from_declaration(
                    &ast::DeclarationShape(ast::Declaration::Module {
                        name: Storage(Visibility::Public, String::from("foo")),
                        value: ast::ModuleShape(Module {
                            imports: vec![],
                            declarations: vec![
                                ast::DeclarationShape(ast::Declaration::Constant {
                                    name: Storage(Visibility::Public, String::from("bar")),
                                    value_type: None,
                                    value: ast::ExpressionShape(ast::Expression::Primitive(
                                        ast::Primitive::Nil
                                    ))
                                }),
                                ast::DeclarationShape(ast::Declaration::Constant {
                                    name: Storage(Visibility::Private, String::from("fizz")),
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
                    String::from("foo"),
                    Expression::Closure(vec![
                        Statement::Variable(String::from("bar"), Expression::Null),
                        Statement::Variable(String::from("fizz"), Expression::Null),
                        Statement::Return(Some(Expression::Object(vec![(
                            String::from("bar"),
                            Expression::Identifier(String::from("bar"))
                        )]))),
                    ])
                )]
            )
        }
    }
}
