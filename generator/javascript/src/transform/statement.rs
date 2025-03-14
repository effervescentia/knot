use crate::{
    ast::{Expression, Statement},
    knot, Options,
};
use kore::{internal, str};

#[allow(clippy::multiple_inherent_impl)]
impl Statement {
    pub fn from_statement<Library>(
        value: &knot::shape::Statement,
        is_last: bool,
        opts: &Options<Library>,
    ) -> Vec<Self> {
        match &value.0 {
            knot::Statement::Expression(x) => {
                if is_last {
                    vec![Self::Return(Some(Expression::from_expression(x, opts)))]
                } else {
                    vec![Self::Expression(Expression::from_expression(x, opts))]
                }
            }

            knot::Statement::Variable(name, x) => {
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

    pub fn from_declaration<Library>(
        path_to_root: &str,
        value: &knot::shape::Declaration,
        opts: &Options<Library>,
    ) -> Vec<Self>
    where
        Library: internal::PlatformLibrary,
    {
        match &value.0 {
            knot::Declaration::TypeAlias { .. } => vec![],

            knot::Declaration::Enumerated {
                storage: knot::Storage { binding, .. },
                variants,
            } => vec![Self::Variable(
                binding.clone(),
                Expression::Object(
                    variants
                        .iter()
                        .map(|(variant_name, variant_parameters)| {
                            if variant_parameters.is_empty() {
                                return (
                                    variant_name.clone(),
                                    Expression::Array(vec![Expression::Function(
                                        Some(variant_name.clone()),
                                        vec![],
                                        vec![],
                                    )]),
                                );
                            }

                            let parameters = variant_parameters
                                .iter()
                                .enumerate()
                                .map(|(index, _)| format!("$param_{}", index))
                                .collect::<Vec<_>>();

                            let results = [
                                vec![Expression::PropertyAccess(
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

            knot::Declaration::Constant {
                storage: knot::Storage { binding, .. },
                value,
                ..
            } => vec![Self::Variable(
                binding.clone(),
                Expression::from_expression(value, opts),
            )],

            knot::Declaration::Function {
                storage: knot::Storage { binding, .. },
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
                                        Box::new(Expression::Identifier(str!(
                                            "$knot.util.defaultParameter"
                                        ))),
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

            knot::Declaration::View {
                storage: knot::Storage { binding, .. },
                parameters,
                body,
                ..
            } => {
                let statements = [
                    parameters
                        .iter()
                        .map(|parameter| {
                            if let Some(default) = &parameter.0.default_value {
                                Self::Assignment(
                                    Expression::Identifier(parameter.0.binding.clone()),
                                    Expression::FunctionCall(
                                        Box::new(Expression::Identifier(str!(
                                            "$knot.util.defaultParameter"
                                        ))),
                                        vec![
                                            Expression::PropertyAccess(
                                                Box::new(Expression::Identifier(str!("$props"))),
                                                parameter.0.binding.clone(),
                                            ),
                                            Expression::from_expression(default, opts),
                                        ],
                                    ),
                                )
                            } else {
                                Self::Variable(
                                    parameter.0.binding.clone(),
                                    Expression::PropertyAccess(
                                        Box::new(Expression::Identifier(str!("$props"))),
                                        parameter.0.binding.clone(),
                                    ),
                                )
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
                    Some(binding.clone()),
                    (!parameters.is_empty())
                        .then_some(vec![str!("$props")])
                        .unwrap_or_default(),
                    statements,
                ))]
            }

            knot::Declaration::Module {
                storage: knot::Storage { binding, .. },
                value,
            } => {
                let statements = [
                    Self::from_module(path_to_root, value, opts),
                    vec![Self::Return(Some(Expression::Object(
                        value
                            .0
                            .declarations
                            .iter()
                            .filter(|x| x.0.is_public())
                            .map(|x| {
                                (
                                    x.0.binding().clone(),
                                    Expression::Identifier(x.0.binding().clone()),
                                )
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
    pub fn from_import<Library>(
        path_to_root: &str,
        knot::shape::Import(import): &knot::shape::Import,
        opts: &Options<Library>,
    ) -> Vec<Self>
    where
        Library: internal::PlatformLibrary,
    {
        let (namespace, alias) = opts.resolver.resolve(path_to_root, import);

        vec![Self::module_import(&namespace, &alias, opts)]
    }

    pub fn from_module<Library>(
        path_to_root: &str,
        value: &knot::shape::Module,
        opts: &Options<Library>,
    ) -> Vec<Self>
    where
        Library: internal::PlatformLibrary,
    {
        let knot::Module {
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
    use crate::{ast::Statement, test::MOCK_OPTIONS};
    use kore::str;
    use lang::ast;

    mod statement {
        use super::*;

        #[test]
        fn expression() {
            let ast = Statement::from_statement(
                &ast::shape::Statement(ast::Statement::Expression(ast::shape::Expression(
                    ast::Expression::Primitive(ast::Primitive::Nil),
                ))),
                false,
                &MOCK_OPTIONS,
            );

            insta::assert_debug_snapshot!(ast);
        }

        #[test]
        fn last_expression() {
            let ast = Statement::from_statement(
                &ast::shape::Statement(ast::Statement::Expression(ast::shape::Expression(
                    ast::Expression::Primitive(ast::Primitive::Nil),
                ))),
                true,
                &MOCK_OPTIONS,
            );

            insta::assert_debug_snapshot!(ast);
        }

        #[test]
        fn variable() {
            let ast = Statement::from_statement(
                &ast::shape::Statement(ast::Statement::Variable(
                    str!("foo"),
                    ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil)),
                )),
                false,
                &MOCK_OPTIONS,
            );

            insta::assert_debug_snapshot!(ast);
        }

        #[test]
        fn last_variable() {
            let ast = Statement::from_statement(
                &ast::shape::Statement(ast::Statement::Variable(
                    str!("foo"),
                    ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil)),
                )),
                true,
                &MOCK_OPTIONS,
            );

            insta::assert_debug_snapshot!(ast);
        }
    }

    mod declaration {
        use super::*;
        use lang::ast;

        #[test]
        fn type_alias() {
            let ast = Statement::from_declaration(
                ".",
                &ast::shape::Declaration(ast::Declaration::TypeAlias {
                    storage: ast::Storage::public(str!("foo")),
                    value: ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                        ast::TypePrimitive::Nil,
                    )),
                }),
                &MOCK_OPTIONS,
            );

            insta::assert_debug_snapshot!(ast);
        }

        #[test]
        fn enumerated() {
            let ast = Statement::from_declaration(
                ".",
                &ast::shape::Declaration(ast::Declaration::Enumerated {
                    storage: ast::Storage::public(str!("foo")),
                    variants: vec![
                        (
                            str!("Bar"),
                            vec![ast::shape::TypeExpression(ast::TypeExpression::Primitive(
                                ast::TypePrimitive::Nil,
                            ))],
                        ),
                        (str!("Fizz"), vec![]),
                    ],
                }),
                &MOCK_OPTIONS,
            );

            insta::assert_debug_snapshot!(ast);
        }

        #[test]
        fn constant() {
            let ast = Statement::from_declaration(
                ".",
                &ast::shape::Declaration(ast::Declaration::constant(
                    ast::Storage::public(str!("foo")),
                    None,
                    ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil)),
                )),
                &MOCK_OPTIONS,
            );

            insta::assert_debug_snapshot!(ast);
        }

        #[test]
        fn function() {
            let ast = Statement::from_declaration(
                ".",
                &ast::shape::Declaration(ast::Declaration::Function {
                    storage: ast::Storage::public(str!("foo")),
                    parameters: vec![
                        ast::shape::Parameter(ast::Parameter::new(str!("bar"), None, None)),
                        ast::shape::Parameter(ast::Parameter::new(
                            str!("fizz"),
                            None,
                            Some(ast::shape::Expression(ast::Expression::Primitive(
                                ast::Primitive::Boolean(true),
                            ))),
                        )),
                    ],
                    body_type: None,
                    body: ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil)),
                }),
                &MOCK_OPTIONS,
            );

            insta::assert_debug_snapshot!(ast);
        }

        #[test]
        fn function_closure_body() {
            let ast = Statement::from_declaration(
                ".",
                &ast::shape::Declaration(ast::Declaration::Function {
                    storage: ast::Storage::public(str!("foo")),
                    parameters: vec![],
                    body_type: None,
                    body: ast::shape::Expression(ast::Expression::Closure(vec![
                        ast::shape::Statement(ast::Statement::Variable(
                            str!("bar"),
                            ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil)),
                        )),
                        ast::shape::Statement(ast::Statement::Expression(ast::shape::Expression(
                            ast::Expression::Identifier(str!("bar")),
                        ))),
                    ])),
                }),
                &MOCK_OPTIONS,
            );

            insta::assert_debug_snapshot!(ast);
        }

        #[test]
        fn view() {
            let ast = Statement::from_declaration(
                ".",
                &ast::shape::Declaration(ast::Declaration::View {
                    storage: ast::Storage::public(str!("foo")),
                    parameters: vec![
                        ast::shape::Parameter(ast::Parameter::new(str!("bar"), None, None)),
                        ast::shape::Parameter(ast::Parameter::new(
                            str!("fizz"),
                            None,
                            Some(ast::shape::Expression(ast::Expression::Primitive(
                                ast::Primitive::Boolean(true),
                            ))),
                        )),
                    ],
                    body: ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil)),
                }),
                &MOCK_OPTIONS,
            );

            insta::assert_debug_snapshot!(ast);
        }

        #[test]
        fn module() {
            let ast = Statement::from_declaration(
                ".",
                &ast::shape::Declaration(ast::Declaration::Module {
                    storage: ast::Storage::public(str!("foo")),
                    value: ast::shape::Module(ast::Module {
                        imports: vec![],
                        declarations: vec![
                            ast::shape::Declaration(ast::Declaration::constant(
                                ast::Storage::public(str!("bar")),
                                None,
                                ast::shape::Expression(ast::Expression::Primitive(
                                    ast::Primitive::Nil,
                                )),
                            )),
                            ast::shape::Declaration(ast::Declaration::constant(
                                ast::Storage::public(str!("fizz")),
                                None,
                                ast::shape::Expression(ast::Expression::Primitive(
                                    ast::Primitive::Nil,
                                )),
                            )),
                        ],
                    }),
                }),
                &MOCK_OPTIONS,
            );

            insta::assert_debug_snapshot!(ast);
        }
    }
}
