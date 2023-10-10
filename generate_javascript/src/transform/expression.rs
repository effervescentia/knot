use crate::{
    javascript::{Expression, Statement},
    Options,
};
use knot_language::ast::{self, ExpressionShape, KSXShape, Primitive};

impl Expression {
    pub fn from_expression(value: &ExpressionShape, opts: &Options) -> Self {
        match &value.0 {
            ast::Expression::Primitive(x) => match x {
                Primitive::Nil => Self::Null,

                Primitive::Boolean(x) => Self::Boolean(*x),

                Primitive::Integer(x) => Self::Number(format!("{x}")),

                Primitive::Float(x, precision) => {
                    Self::Number(format!("{x:.0$}", *precision as usize))
                }

                Primitive::String(x) => Self::String(x.clone()),
            },

            ast::Expression::Identifier(x) => Self::Identifier(x.clone()),

            ast::Expression::Group(x) => Self::Group(Box::new(Self::from_expression(&x, opts))),

            ast::Expression::Closure(xs) if xs.is_empty() => Self::Null,
            ast::Expression::Closure(xs) => {
                let statements = xs
                    .iter()
                    .enumerate()
                    .flat_map(|(index, x)| {
                        if index == xs.len() - 1 {
                            Statement::from_last_statement(x, opts)
                        } else {
                            Statement::from_statement(x, opts)
                        }
                    })
                    .collect::<Vec<_>>();

                Expression::Closure(statements)
            }

            ast::Expression::UnaryOperation(op, x) => match op {
                ast::UnaryOperator::Not => {
                    Self::UnaryOperation("!", Box::new(Self::from_expression(&x, opts)))
                }

                ast::UnaryOperator::Negate => {
                    Self::UnaryOperation("-", Box::new(Self::from_expression(&x, opts)))
                }

                ast::UnaryOperator::Absolute => {
                    Self::call_global("Math.abs", vec![Self::from_expression(&x, opts)])
                }
            },

            ast::Expression::BinaryOperation(op, lhs, rhs) => {
                let binary_op = |op| {
                    Self::BinaryOperation(
                        op,
                        Box::new(Self::from_expression(&lhs, opts)),
                        Box::new(Self::from_expression(&rhs, opts)),
                    )
                };

                match op {
                    ast::BinaryOperator::And => binary_op("&&"),
                    ast::BinaryOperator::Or => binary_op("||"),

                    ast::BinaryOperator::Equal => binary_op("==="),
                    ast::BinaryOperator::NotEqual => binary_op("!=="),

                    ast::BinaryOperator::LessThan => binary_op("<"),
                    ast::BinaryOperator::LessThanOrEqual => binary_op("<="),
                    ast::BinaryOperator::GreaterThan => binary_op(">"),
                    ast::BinaryOperator::GreaterThanOrEqual => binary_op(">="),

                    ast::BinaryOperator::Add => binary_op("+"),
                    ast::BinaryOperator::Subtract => binary_op("-"),
                    ast::BinaryOperator::Multiply => binary_op("*"),
                    ast::BinaryOperator::Divide => binary_op("/"),

                    ast::BinaryOperator::Exponent => Self::call_global(
                        "Math.pow",
                        vec![
                            Self::from_expression(&lhs, opts),
                            Self::from_expression(&rhs, opts),
                        ],
                    ),
                }
            }

            ast::Expression::DotAccess(lhs, rhs) => {
                Self::DotAccess(Box::new(Self::from_expression(&lhs, opts)), rhs.clone())
            }

            ast::Expression::FunctionCall(x, arguments) => Self::FunctionCall(
                Box::new(Self::from_expression(&x, opts)),
                arguments
                    .iter()
                    .map(|x| Self::from_expression(x, opts))
                    .collect(),
            ),

            ast::Expression::Style(xs) => Self::FunctionCall(
                Box::new(Self::plugin("style", "create")),
                vec![Self::Object(
                    xs.iter()
                        .map(|(key, value)| (key.clone(), Self::from_expression(value, opts)))
                        .collect(),
                )],
            ),

            ast::Expression::KSX(x) => Self::from_ksx(&x, opts),
        }
    }

    pub fn from_ksx(value: &KSXShape, opts: &Options) -> Self {
        fn element_name(name: String) -> Expression {
            let first_char = name.chars().next().unwrap();

            if first_char.is_lowercase() {
                Expression::String(name)
            } else {
                Expression::Identifier(name)
            }
        }

        match &value.0 {
            ast::KSX::Text(x) => Self::String(x.clone()),

            ast::KSX::Inline(x) => Self::from_expression(&x, opts),

            ast::KSX::Fragment(xs) => Self::FunctionCall(
                Box::new(Self::plugin("ksx", "createFragment")),
                xs.iter().map(|x| Self::from_ksx(x, opts)).collect(),
            ),

            ast::KSX::ClosedElement(tag, attributes) => {
                let name_arg = element_name(tag.clone());

                Self::FunctionCall(
                    Box::new(Self::plugin("ksx", "createElement")),
                    if attributes.is_empty() {
                        vec![name_arg]
                    } else {
                        vec![name_arg, Self::from_attributes(attributes, opts)]
                    },
                )
            }

            ast::KSX::OpenElement(start_tag, attributes, children, ..) => {
                let name_arg = element_name(start_tag.clone());

                Self::FunctionCall(
                    Box::new(Self::plugin("ksx", "createElement")),
                    if attributes.is_empty() && children.is_empty() {
                        vec![name_arg]
                    } else {
                        vec![
                            vec![name_arg, Self::from_attributes(attributes, opts)],
                            children.iter().map(|x| Self::from_ksx(x, opts)).collect(),
                        ]
                        .concat()
                    },
                )
            }
        }
    }

    pub fn from_attributes(
        xs: &Vec<(String, Option<ast::ExpressionShape>)>,
        opts: &Options,
    ) -> Self {
        if xs.is_empty() {
            return Self::Null;
        }

        Self::Object(
            xs.iter()
                .map(|(name, x)| {
                    (
                        name.clone(),
                        match x {
                            Some(x) => Self::from_expression(x, opts),
                            None => Self::Identifier(name.clone()),
                        },
                    )
                })
                .collect(),
        )
    }
}

#[cfg(test)]
mod tests {
    use crate::{
        javascript::{Expression, Statement},
        Mode, Options,
    };
    use knot_language::ast;

    const OPTIONS: Options = Options { mode: Mode::Prod };

    mod expression {
        use super::*;

        #[test]
        fn primitive_nil() {
            assert_eq!(
                Expression::from_expression(
                    &ast::ExpressionShape(ast::Expression::Primitive(ast::Primitive::Nil)),
                    &OPTIONS
                ),
                Expression::Null
            );
        }

        #[test]
        fn primitive_boolean() {
            assert_eq!(
                Expression::from_expression(
                    &ast::ExpressionShape(ast::Expression::Primitive(ast::Primitive::Boolean(
                        true
                    ))),
                    &OPTIONS
                ),
                Expression::Boolean(true)
            );
            assert_eq!(
                Expression::from_expression(
                    &ast::ExpressionShape(ast::Expression::Primitive(ast::Primitive::Boolean(
                        false
                    ))),
                    &OPTIONS
                ),
                Expression::Boolean(false)
            );
        }

        #[test]
        fn primitive_integer() {
            assert_eq!(
                Expression::from_expression(
                    &ast::ExpressionShape(ast::Expression::Primitive(ast::Primitive::Integer(123))),
                    &OPTIONS
                ),
                Expression::Number(String::from("123"))
            );
        }

        #[test]
        fn primitive_float() {
            assert_eq!(
                Expression::from_expression(
                    &ast::ExpressionShape(ast::Expression::Primitive(ast::Primitive::Float(
                        45.67, 2
                    ))),
                    &OPTIONS
                ),
                Expression::Number(String::from("45.67"))
            );
        }

        #[test]
        fn primitive_string() {
            assert_eq!(
                Expression::from_expression(
                    &ast::ExpressionShape(ast::Expression::Primitive(ast::Primitive::String(
                        String::from("foo")
                    ))),
                    &OPTIONS
                ),
                Expression::String(String::from("foo"))
            );
        }

        #[test]
        fn identifier() {
            assert_eq!(
                Expression::from_expression(
                    &ast::ExpressionShape(ast::Expression::Identifier(String::from("foo"))),
                    &OPTIONS
                ),
                Expression::Identifier(String::from("foo"))
            );
        }

        #[test]
        fn group() {
            assert_eq!(
                Expression::from_expression(
                    &ast::ExpressionShape(ast::Expression::Group(Box::new(ast::ExpressionShape(
                        ast::Expression::Primitive(ast::Primitive::Nil)
                    )))),
                    &OPTIONS
                ),
                Expression::Group(Box::new(Expression::Null))
            );
        }

        #[test]
        fn empty_closure() {
            assert_eq!(
                Expression::from_expression(
                    &ast::ExpressionShape(ast::Expression::Closure(vec![])),
                    &OPTIONS
                ),
                Expression::Null
            );
        }

        #[test]
        fn closure_with_last_expression() {
            assert_eq!(
                Expression::from_expression(
                    &ast::ExpressionShape(ast::Expression::Closure(vec![
                        ast::StatementShape(ast::Statement::Variable(
                            String::from("foo"),
                            ast::ExpressionShape(ast::Expression::Primitive(ast::Primitive::Nil))
                        )),
                        ast::StatementShape(ast::Statement::Expression(ast::ExpressionShape(
                            ast::Expression::Primitive(ast::Primitive::Boolean(true))
                        )))
                    ])),
                    &OPTIONS
                ),
                Expression::Closure(vec![
                    Statement::Variable(String::from("foo"), Expression::Null),
                    Statement::Return(Some(Expression::Boolean(true)))
                ])
            );
        }

        #[test]
        fn closure_with_last_variable() {
            assert_eq!(
                Expression::from_expression(
                    &ast::ExpressionShape(ast::Expression::Closure(vec![
                        ast::StatementShape(ast::Statement::Expression(ast::ExpressionShape(
                            ast::Expression::Primitive(ast::Primitive::Boolean(true))
                        ))),
                        ast::StatementShape(ast::Statement::Variable(
                            String::from("foo"),
                            ast::ExpressionShape(ast::Expression::Primitive(ast::Primitive::Nil))
                        ))
                    ])),
                    &OPTIONS
                ),
                Expression::Closure(vec![
                    Statement::Expression(Expression::Boolean(true)),
                    Statement::Variable(String::from("foo"), Expression::Null),
                    Statement::Return(None)
                ])
            );
        }

        #[test]
        fn unary_operation() {
            let operation = |op| {
                Expression::from_expression(
                    &ast::ExpressionShape(ast::Expression::UnaryOperation(
                        op,
                        Box::new(ast::ExpressionShape(ast::Expression::Primitive(
                            ast::Primitive::Nil,
                        ))),
                    )),
                    &OPTIONS,
                )
            };

            assert_eq!(
                operation(ast::UnaryOperator::Not),
                Expression::UnaryOperation("!", Box::new(Expression::Null))
            );
            assert_eq!(
                operation(ast::UnaryOperator::Negate),
                Expression::UnaryOperation("-", Box::new(Expression::Null))
            );
        }

        #[test]
        fn unary_absolute_operation() {
            assert_eq!(
                Expression::from_expression(
                    &ast::ExpressionShape(ast::Expression::UnaryOperation(
                        ast::UnaryOperator::Absolute,
                        Box::new(ast::ExpressionShape(ast::Expression::Primitive(
                            ast::Primitive::Nil
                        )))
                    )),
                    &OPTIONS
                ),
                Expression::FunctionCall(
                    Box::new(Expression::Identifier(String::from("Math.abs"))),
                    vec![Expression::Null]
                ),
            );
        }

        #[test]
        fn binary_operation() {
            let knot_operation = |op| {
                Expression::from_expression(
                    &ast::ExpressionShape(ast::Expression::BinaryOperation(
                        op,
                        Box::new(ast::ExpressionShape(ast::Expression::Identifier(
                            String::from("lhs"),
                        ))),
                        Box::new(ast::ExpressionShape(ast::Expression::Identifier(
                            String::from("rhs"),
                        ))),
                    )),
                    &OPTIONS,
                )
            };
            let js_operation = |op| {
                Expression::BinaryOperation(
                    op,
                    Box::new(Expression::Identifier(String::from("lhs"))),
                    Box::new(Expression::Identifier(String::from("rhs"))),
                )
            };

            assert_eq!(knot_operation(ast::BinaryOperator::And), js_operation("&&"));
            assert_eq!(knot_operation(ast::BinaryOperator::Or), js_operation("||"),);

            assert_eq!(
                knot_operation(ast::BinaryOperator::Equal),
                js_operation("==="),
            );
            assert_eq!(
                knot_operation(ast::BinaryOperator::NotEqual),
                js_operation("!=="),
            );

            assert_eq!(
                knot_operation(ast::BinaryOperator::GreaterThan),
                js_operation(">"),
            );
            assert_eq!(
                knot_operation(ast::BinaryOperator::GreaterThanOrEqual),
                js_operation(">="),
            );
            assert_eq!(
                knot_operation(ast::BinaryOperator::LessThan),
                js_operation("<"),
            );
            assert_eq!(
                knot_operation(ast::BinaryOperator::LessThanOrEqual),
                js_operation("<="),
            );

            assert_eq!(knot_operation(ast::BinaryOperator::Add), js_operation("+"),);
            assert_eq!(
                knot_operation(ast::BinaryOperator::Subtract),
                js_operation("-"),
            );
            assert_eq!(
                knot_operation(ast::BinaryOperator::Multiply),
                js_operation("*"),
            );
            assert_eq!(
                knot_operation(ast::BinaryOperator::Divide),
                js_operation("/"),
            );
        }

        #[test]
        fn binary_exponent_operation() {
            assert_eq!(
                Expression::from_expression(
                    &ast::ExpressionShape(ast::Expression::BinaryOperation(
                        ast::BinaryOperator::Exponent,
                        Box::new(ast::ExpressionShape(ast::Expression::Primitive(
                            ast::Primitive::Nil
                        ))),
                        Box::new(ast::ExpressionShape(ast::Expression::Primitive(
                            ast::Primitive::Nil
                        )))
                    )),
                    &OPTIONS
                ),
                Expression::FunctionCall(
                    Box::new(Expression::Identifier(String::from("Math.pow"))),
                    vec![Expression::Null, Expression::Null]
                ),
            );
        }

        #[test]
        fn dot_access() {
            assert_eq!(
                Expression::from_expression(
                    &ast::ExpressionShape(ast::Expression::DotAccess(
                        Box::new(ast::ExpressionShape(ast::Expression::Primitive(
                            ast::Primitive::Nil
                        ))),
                        String::from("foo")
                    )),
                    &OPTIONS
                ),
                Expression::DotAccess(Box::new(Expression::Null), String::from("foo")),
            );
        }

        #[test]
        fn function_call() {
            assert_eq!(
                Expression::from_expression(
                    &ast::ExpressionShape(ast::Expression::FunctionCall(
                        Box::new(ast::ExpressionShape(ast::Expression::Primitive(
                            ast::Primitive::Nil
                        ))),
                        vec![ast::ExpressionShape(ast::Expression::Primitive(
                            ast::Primitive::Nil
                        ))]
                    )),
                    &OPTIONS
                ),
                Expression::FunctionCall(Box::new(Expression::Null), vec![Expression::Null]),
            );
        }

        #[test]
        fn style() {
            assert_eq!(
                Expression::from_expression(
                    &ast::ExpressionShape(ast::Expression::Style(vec![(
                        String::from("foo"),
                        ast::ExpressionShape(ast::Expression::Primitive(ast::Primitive::Nil))
                    )])),
                    &OPTIONS
                ),
                Expression::FunctionCall(
                    Box::new(Expression::FunctionCall(
                        Box::new(Expression::Identifier(String::from("$knot.plugin.get"))),
                        vec![
                            Expression::String(String::from("style")),
                            Expression::String(String::from("create")),
                            Expression::String(String::from("1.0")),
                        ]
                    )),
                    vec![Expression::Object(vec![(
                        String::from("foo"),
                        Expression::Null
                    )])]
                ),
            );
        }

        #[test]
        fn ksx() {
            assert_eq!(
                Expression::from_expression(
                    &ast::ExpressionShape(ast::Expression::KSX(Box::new(ast::KSXShape(
                        ast::KSX::ClosedElement(String::from("Foo"), vec![])
                    )))),
                    &OPTIONS
                ),
                Expression::FunctionCall(
                    Box::new(Expression::FunctionCall(
                        Box::new(Expression::Identifier(String::from("$knot.plugin.get"))),
                        vec![
                            Expression::String(String::from("ksx")),
                            Expression::String(String::from("createElement")),
                            Expression::String(String::from("1.0")),
                        ]
                    )),
                    vec![Expression::Identifier(String::from("Foo"))]
                ),
            );
        }
    }

    mod ksx {
        use super::*;

        #[test]
        fn text() {
            assert_eq!(
                Expression::from_ksx(
                    &ast::KSXShape(ast::KSX::Text(String::from("foo"))),
                    &OPTIONS
                ),
                Expression::String(String::from("foo"))
            );
        }

        #[test]
        fn inline() {
            assert_eq!(
                Expression::from_ksx(
                    &ast::KSXShape(ast::KSX::Inline(ast::ExpressionShape(
                        ast::Expression::Primitive(ast::Primitive::Nil)
                    ))),
                    &OPTIONS
                ),
                Expression::Null
            );
        }

        #[test]
        fn fragment() {
            assert_eq!(
                Expression::from_ksx(
                    &ast::KSXShape(ast::KSX::Fragment(vec![
                        ast::KSXShape(ast::KSX::Text(String::from("foo"))),
                        ast::KSXShape(ast::KSX::Text(String::from("bar"))),
                    ])),
                    &OPTIONS
                ),
                Expression::FunctionCall(
                    Box::new(Expression::FunctionCall(
                        Box::new(Expression::Identifier(String::from("$knot.plugin.get"))),
                        vec![
                            Expression::String(String::from("ksx")),
                            Expression::String(String::from("createFragment")),
                            Expression::String(String::from("1.0")),
                        ]
                    )),
                    vec![
                        Expression::String(String::from("foo")),
                        Expression::String(String::from("bar")),
                    ]
                )
            );
        }

        #[test]
        fn empty_fragment() {
            assert_eq!(
                Expression::from_ksx(&ast::KSXShape(ast::KSX::Fragment(vec![])), &OPTIONS),
                Expression::FunctionCall(
                    Box::new(Expression::FunctionCall(
                        Box::new(Expression::Identifier(String::from("$knot.plugin.get"))),
                        vec![
                            Expression::String(String::from("ksx")),
                            Expression::String(String::from("createFragment")),
                            Expression::String(String::from("1.0")),
                        ]
                    )),
                    vec![]
                )
            );
        }

        #[test]
        fn closed_element() {
            assert_eq!(
                Expression::from_ksx(
                    &ast::KSXShape(ast::KSX::ClosedElement(
                        String::from("Foo"),
                        vec![
                            (String::from("bar"), None),
                            (
                                String::from("fizz"),
                                Some(ast::ExpressionShape(ast::Expression::Primitive(
                                    ast::Primitive::Nil
                                )))
                            ),
                        ]
                    )),
                    &OPTIONS
                ),
                Expression::FunctionCall(
                    Box::new(Expression::FunctionCall(
                        Box::new(Expression::Identifier(String::from("$knot.plugin.get"))),
                        vec![
                            Expression::String(String::from("ksx")),
                            Expression::String(String::from("createElement")),
                            Expression::String(String::from("1.0")),
                        ]
                    )),
                    vec![
                        Expression::Identifier(String::from("Foo")),
                        Expression::Object(vec![
                            (
                                String::from("bar"),
                                Expression::Identifier(String::from("bar"))
                            ),
                            (String::from("fizz"), Expression::Null)
                        ])
                    ]
                )
            );
        }

        #[test]
        fn closed_element_no_attributes() {
            assert_eq!(
                Expression::from_ksx(
                    &ast::KSXShape(ast::KSX::ClosedElement(String::from("Foo"), vec![])),
                    &OPTIONS
                ),
                Expression::FunctionCall(
                    Box::new(Expression::FunctionCall(
                        Box::new(Expression::Identifier(String::from("$knot.plugin.get"))),
                        vec![
                            Expression::String(String::from("ksx")),
                            Expression::String(String::from("createElement")),
                            Expression::String(String::from("1.0")),
                        ]
                    )),
                    vec![Expression::Identifier(String::from("Foo"))]
                )
            );
        }

        #[test]
        fn open_element() {
            assert_eq!(
                Expression::from_ksx(
                    &ast::KSXShape(ast::KSX::OpenElement(
                        String::from("Foo"),
                        vec![
                            (String::from("bar"), None),
                            (
                                String::from("fizz"),
                                Some(ast::ExpressionShape(ast::Expression::Primitive(
                                    ast::Primitive::Nil
                                )))
                            ),
                        ],
                        vec![
                            ast::KSXShape(ast::KSX::Text(String::from("foo"))),
                            ast::KSXShape(ast::KSX::Text(String::from("bar"))),
                        ],
                        String::from("Foo"),
                    )),
                    &OPTIONS
                ),
                Expression::FunctionCall(
                    Box::new(Expression::FunctionCall(
                        Box::new(Expression::Identifier(String::from("$knot.plugin.get"))),
                        vec![
                            Expression::String(String::from("ksx")),
                            Expression::String(String::from("createElement")),
                            Expression::String(String::from("1.0")),
                        ]
                    )),
                    vec![
                        Expression::Identifier(String::from("Foo")),
                        Expression::Object(vec![
                            (
                                String::from("bar"),
                                Expression::Identifier(String::from("bar"))
                            ),
                            (String::from("fizz"), Expression::Null)
                        ]),
                        Expression::String(String::from("foo")),
                        Expression::String(String::from("bar")),
                    ]
                )
            );
        }

        #[test]
        fn open_element_no_attributes() {
            assert_eq!(
                Expression::from_ksx(
                    &ast::KSXShape(ast::KSX::OpenElement(
                        String::from("Foo"),
                        vec![],
                        vec![
                            ast::KSXShape(ast::KSX::Text(String::from("foo"))),
                            ast::KSXShape(ast::KSX::Text(String::from("bar"))),
                        ],
                        String::from("Foo"),
                    )),
                    &OPTIONS
                ),
                Expression::FunctionCall(
                    Box::new(Expression::FunctionCall(
                        Box::new(Expression::Identifier(String::from("$knot.plugin.get"))),
                        vec![
                            Expression::String(String::from("ksx")),
                            Expression::String(String::from("createElement")),
                            Expression::String(String::from("1.0")),
                        ]
                    )),
                    vec![
                        Expression::Identifier(String::from("Foo")),
                        Expression::Null,
                        Expression::String(String::from("foo")),
                        Expression::String(String::from("bar")),
                    ]
                )
            );
        }

        #[test]
        fn open_element_no_children() {
            assert_eq!(
                Expression::from_ksx(
                    &ast::KSXShape(ast::KSX::OpenElement(
                        String::from("Foo"),
                        vec![
                            (String::from("bar"), None),
                            (
                                String::from("fizz"),
                                Some(ast::ExpressionShape(ast::Expression::Primitive(
                                    ast::Primitive::Nil
                                )))
                            ),
                        ],
                        vec![],
                        String::from("Foo"),
                    )),
                    &OPTIONS
                ),
                Expression::FunctionCall(
                    Box::new(Expression::FunctionCall(
                        Box::new(Expression::Identifier(String::from("$knot.plugin.get"))),
                        vec![
                            Expression::String(String::from("ksx")),
                            Expression::String(String::from("createElement")),
                            Expression::String(String::from("1.0")),
                        ]
                    )),
                    vec![
                        Expression::Identifier(String::from("Foo")),
                        Expression::Object(vec![
                            (
                                String::from("bar"),
                                Expression::Identifier(String::from("bar"))
                            ),
                            (String::from("fizz"), Expression::Null)
                        ]),
                    ]
                )
            );
        }
    }
}
