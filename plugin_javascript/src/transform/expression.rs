use crate::{
    javascript::{Expression, Statement},
    Options,
};
use kore::invariant;
use lang::ast;

#[allow(clippy::multiple_inherent_impl)]
impl Expression {
    pub fn from_expression(value: &ast::shape::Expression, opts: &Options) -> Self {
        match &value.0 {
            ast::Expression::Primitive(x) => match x {
                ast::Primitive::Nil => Self::Null,

                ast::Primitive::Boolean(x) => Self::Boolean(*x),

                ast::Primitive::Integer(x) => Self::Number(format!("{x}")),

                ast::Primitive::Float(x, precision) => {
                    Self::Number(format!("{x:.0$}", *precision as usize))
                }

                ast::Primitive::String(x) => Self::String(x.clone()),
            },

            ast::Expression::Identifier(x) => Self::Identifier(x.clone()),

            ast::Expression::Group(x) => Self::Group(Box::new(Self::from_expression(x, opts))),

            ast::Expression::Closure(xs) if xs.is_empty() => Self::Null,
            ast::Expression::Closure(xs) => {
                let statements = xs
                    .iter()
                    .enumerate()
                    .flat_map(|(index, x)| {
                        Statement::from_statement(x, index == xs.len() - 1, opts)
                    })
                    .collect::<Vec<_>>();

                Self::Closure(statements)
            }

            ast::Expression::UnaryOperation(op, x) => match op {
                ast::UnaryOperator::Not => {
                    Self::UnaryOperation("!", Box::new(Self::from_expression(x, opts)))
                }

                ast::UnaryOperator::Negate => {
                    Self::UnaryOperation("-", Box::new(Self::from_expression(x, opts)))
                }

                ast::UnaryOperator::Absolute => {
                    Self::call_global("Math.abs", vec![Self::from_expression(x, opts)])
                }
            },

            ast::Expression::BinaryOperation(op, lhs, rhs) => {
                let binary_op = |op| {
                    Self::BinaryOperation(
                        op,
                        Box::new(Self::from_expression(lhs, opts)),
                        Box::new(Self::from_expression(rhs, opts)),
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
                            Self::from_expression(lhs, opts),
                            Self::from_expression(rhs, opts),
                        ],
                    ),
                }
            }

            ast::Expression::PropertyAccess(lhs, rhs) => {
                Self::DotAccess(Box::new(Self::from_expression(lhs, opts)), rhs.clone())
            }

            ast::Expression::FunctionCall(x, arguments) => Self::FunctionCall(
                Box::new(Self::from_expression(x, opts)),
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

            ast::Expression::Component(x) => Self::from_component(x, opts),
        }
    }

    pub fn from_component(value: &ast::shape::Component, opts: &Options) -> Self {
        fn element_name(name: String) -> Expression {
            let first_char = name
                .chars()
                .next()
                .unwrap_or_else(|| invariant!("element names should be at least 1 character long"));

            if first_char.is_lowercase() {
                Expression::String(name)
            } else {
                Expression::Identifier(name)
            }
        }

        match &value.0 {
            ast::Component::Text(x) => Self::String(x.clone()),

            ast::Component::Expression(x) => Self::from_expression(x, opts),

            ast::Component::Fragment(xs) => Self::FunctionCall(
                Box::new(Self::plugin("ksx", "createFragment")),
                xs.iter().map(|x| Self::from_component(x, opts)).collect(),
            ),

            ast::Component::ClosedElement(tag, attributes) => {
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

            ast::Component::OpenElement {
                start_tag,
                attributes,
                children,
                ..
            } => {
                let name_arg = element_name(start_tag.clone());

                Self::FunctionCall(
                    Box::new(Self::plugin("ksx", "createElement")),
                    if attributes.is_empty() && children.is_empty() {
                        vec![name_arg]
                    } else {
                        [
                            vec![name_arg, Self::from_attributes(attributes, opts)],
                            children
                                .iter()
                                .map(|x| Self::from_component(x, opts))
                                .collect(),
                        ]
                        .concat()
                    },
                )
            }
        }
    }

    pub fn from_attributes(
        xs: &Vec<(String, Option<ast::shape::Expression>)>,
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
        Mode, Module, Options,
    };
    use kore::str;
    use lang::ast;

    const OPTIONS: Options = Options {
        mode: Mode::Prod,
        module: Module::ESM,
    };

    mod expression {
        use super::*;

        #[test]
        fn primitive_nil() {
            assert_eq!(
                Expression::from_expression(
                    &ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil)),
                    &OPTIONS
                ),
                Expression::Null
            );
        }

        #[test]
        fn primitive_boolean() {
            assert_eq!(
                Expression::from_expression(
                    &ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Boolean(
                        true
                    ))),
                    &OPTIONS
                ),
                Expression::Boolean(true)
            );
            assert_eq!(
                Expression::from_expression(
                    &ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Boolean(
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
                    &ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Integer(
                        123
                    ))),
                    &OPTIONS
                ),
                Expression::Number(str!("123"))
            );
        }

        #[test]
        fn primitive_float() {
            assert_eq!(
                Expression::from_expression(
                    &ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Float(
                        45.67, 2
                    ))),
                    &OPTIONS
                ),
                Expression::Number(str!("45.67"))
            );
        }

        #[test]
        fn primitive_string() {
            assert_eq!(
                Expression::from_expression(
                    &ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::String(
                        str!("foo")
                    ))),
                    &OPTIONS
                ),
                Expression::String(str!("foo"))
            );
        }

        #[test]
        fn identifier() {
            assert_eq!(
                Expression::from_expression(
                    &ast::shape::Expression(ast::Expression::Identifier(str!("foo"))),
                    &OPTIONS
                ),
                Expression::Identifier(str!("foo"))
            );
        }

        #[test]
        fn group() {
            assert_eq!(
                Expression::from_expression(
                    &ast::shape::Expression(ast::Expression::Group(Box::new(
                        ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil))
                    ))),
                    &OPTIONS
                ),
                Expression::Group(Box::new(Expression::Null))
            );
        }

        #[test]
        fn empty_closure() {
            assert_eq!(
                Expression::from_expression(
                    &ast::shape::Expression(ast::Expression::Closure(vec![])),
                    &OPTIONS
                ),
                Expression::Null
            );
        }

        #[test]
        fn closure_with_last_expression() {
            assert_eq!(
                Expression::from_expression(
                    &ast::shape::Expression(ast::Expression::Closure(vec![
                        ast::shape::Statement(ast::Statement::Variable(
                            str!("foo"),
                            ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil))
                        )),
                        ast::shape::Statement(ast::Statement::Expression(ast::shape::Expression(
                            ast::Expression::Primitive(ast::Primitive::Boolean(true))
                        )))
                    ])),
                    &OPTIONS
                ),
                Expression::Closure(vec![
                    Statement::Variable(str!("foo"), Expression::Null),
                    Statement::Return(Some(Expression::Boolean(true)))
                ])
            );
        }

        #[test]
        fn closure_with_last_variable() {
            assert_eq!(
                Expression::from_expression(
                    &ast::shape::Expression(ast::Expression::Closure(vec![
                        ast::shape::Statement(ast::Statement::Expression(ast::shape::Expression(
                            ast::Expression::Primitive(ast::Primitive::Boolean(true))
                        ))),
                        ast::shape::Statement(ast::Statement::Variable(
                            str!("foo"),
                            ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil))
                        ))
                    ])),
                    &OPTIONS
                ),
                Expression::Closure(vec![
                    Statement::Expression(Expression::Boolean(true)),
                    Statement::Variable(str!("foo"), Expression::Null),
                    Statement::Return(None)
                ])
            );
        }

        #[test]
        fn unary_operation() {
            let operation = |op| {
                Expression::from_expression(
                    &ast::shape::Expression(ast::Expression::UnaryOperation(
                        op,
                        Box::new(ast::shape::Expression(ast::Expression::Primitive(
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
                    &ast::shape::Expression(ast::Expression::UnaryOperation(
                        ast::UnaryOperator::Absolute,
                        Box::new(ast::shape::Expression(ast::Expression::Primitive(
                            ast::Primitive::Nil
                        )))
                    )),
                    &OPTIONS
                ),
                Expression::FunctionCall(
                    Box::new(Expression::Identifier(str!("Math.abs"))),
                    vec![Expression::Null]
                ),
            );
        }

        #[test]
        fn binary_operation() {
            let knot_operation = |op| {
                Expression::from_expression(
                    &ast::shape::Expression(ast::Expression::BinaryOperation(
                        op,
                        Box::new(ast::shape::Expression(ast::Expression::Identifier(str!(
                            "lhs"
                        )))),
                        Box::new(ast::shape::Expression(ast::Expression::Identifier(str!(
                            "rhs"
                        )))),
                    )),
                    &OPTIONS,
                )
            };
            let js_operation = |op| {
                Expression::BinaryOperation(
                    op,
                    Box::new(Expression::Identifier(str!("lhs"))),
                    Box::new(Expression::Identifier(str!("rhs"))),
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
                    &ast::shape::Expression(ast::Expression::BinaryOperation(
                        ast::BinaryOperator::Exponent,
                        Box::new(ast::shape::Expression(ast::Expression::Primitive(
                            ast::Primitive::Nil
                        ))),
                        Box::new(ast::shape::Expression(ast::Expression::Primitive(
                            ast::Primitive::Nil
                        )))
                    )),
                    &OPTIONS
                ),
                Expression::FunctionCall(
                    Box::new(Expression::Identifier(str!("Math.pow"))),
                    vec![Expression::Null, Expression::Null]
                ),
            );
        }

        #[test]
        fn dot_access() {
            assert_eq!(
                Expression::from_expression(
                    &ast::shape::Expression(ast::Expression::PropertyAccess(
                        Box::new(ast::shape::Expression(ast::Expression::Primitive(
                            ast::Primitive::Nil
                        ))),
                        str!("foo")
                    )),
                    &OPTIONS
                ),
                Expression::DotAccess(Box::new(Expression::Null), str!("foo")),
            );
        }

        #[test]
        fn function_call() {
            assert_eq!(
                Expression::from_expression(
                    &ast::shape::Expression(ast::Expression::FunctionCall(
                        Box::new(ast::shape::Expression(ast::Expression::Primitive(
                            ast::Primitive::Nil
                        ))),
                        vec![ast::shape::Expression(ast::Expression::Primitive(
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
                    &ast::shape::Expression(ast::Expression::Style(vec![(
                        str!("foo"),
                        ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil))
                    )])),
                    &OPTIONS
                ),
                Expression::FunctionCall(
                    Box::new(Expression::FunctionCall(
                        Box::new(Expression::Identifier(str!("$knot.plugin.get"))),
                        vec![
                            Expression::String(str!("style")),
                            Expression::String(str!("create")),
                            Expression::String(str!("1.0")),
                        ]
                    )),
                    vec![Expression::Object(vec![(str!("foo"), Expression::Null)])]
                ),
            );
        }

        #[test]
        fn ksx() {
            assert_eq!(
                Expression::from_expression(
                    &ast::shape::Expression(ast::Expression::Component(Box::new(
                        ast::shape::Component(ast::Component::ClosedElement(str!("Foo"), vec![]))
                    ))),
                    &OPTIONS
                ),
                Expression::FunctionCall(
                    Box::new(Expression::FunctionCall(
                        Box::new(Expression::Identifier(str!("$knot.plugin.get"))),
                        vec![
                            Expression::String(str!("ksx")),
                            Expression::String(str!("createElement")),
                            Expression::String(str!("1.0")),
                        ]
                    )),
                    vec![Expression::Identifier(str!("Foo"))]
                ),
            );
        }
    }

    mod ksx {
        use super::*;

        #[test]
        fn text() {
            assert_eq!(
                Expression::from_component(
                    &ast::shape::Component(ast::Component::Text(str!("foo"))),
                    &OPTIONS
                ),
                Expression::String(str!("foo"))
            );
        }

        #[test]
        fn inline() {
            assert_eq!(
                Expression::from_component(
                    &ast::shape::Component(ast::Component::Expression(ast::shape::Expression(
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
                Expression::from_component(
                    &ast::shape::Component(ast::Component::Fragment(vec![
                        ast::shape::Component(ast::Component::Text(str!("foo"))),
                        ast::shape::Component(ast::Component::Text(str!("bar"))),
                    ])),
                    &OPTIONS
                ),
                Expression::FunctionCall(
                    Box::new(Expression::FunctionCall(
                        Box::new(Expression::Identifier(str!("$knot.plugin.get"))),
                        vec![
                            Expression::String(str!("ksx")),
                            Expression::String(str!("createFragment")),
                            Expression::String(str!("1.0")),
                        ]
                    )),
                    vec![
                        Expression::String(str!("foo")),
                        Expression::String(str!("bar")),
                    ]
                )
            );
        }

        #[test]
        fn empty_fragment() {
            assert_eq!(
                Expression::from_component(
                    &ast::shape::Component(ast::Component::Fragment(vec![])),
                    &OPTIONS
                ),
                Expression::FunctionCall(
                    Box::new(Expression::FunctionCall(
                        Box::new(Expression::Identifier(str!("$knot.plugin.get"))),
                        vec![
                            Expression::String(str!("ksx")),
                            Expression::String(str!("createFragment")),
                            Expression::String(str!("1.0")),
                        ]
                    )),
                    vec![]
                )
            );
        }

        #[test]
        fn closed_element() {
            assert_eq!(
                Expression::from_component(
                    &ast::shape::Component(ast::Component::ClosedElement(
                        str!("Foo"),
                        vec![
                            (str!("bar"), None),
                            (
                                str!("fizz"),
                                Some(ast::shape::Expression(ast::Expression::Primitive(
                                    ast::Primitive::Nil
                                )))
                            ),
                        ]
                    )),
                    &OPTIONS
                ),
                Expression::FunctionCall(
                    Box::new(Expression::FunctionCall(
                        Box::new(Expression::Identifier(str!("$knot.plugin.get"))),
                        vec![
                            Expression::String(str!("ksx")),
                            Expression::String(str!("createElement")),
                            Expression::String(str!("1.0")),
                        ]
                    )),
                    vec![
                        Expression::Identifier(str!("Foo")),
                        Expression::Object(vec![
                            (str!("bar"), Expression::Identifier(str!("bar"))),
                            (str!("fizz"), Expression::Null)
                        ])
                    ]
                )
            );
        }

        #[test]
        fn closed_element_no_attributes() {
            assert_eq!(
                Expression::from_component(
                    &ast::shape::Component(ast::Component::ClosedElement(str!("Foo"), vec![])),
                    &OPTIONS
                ),
                Expression::FunctionCall(
                    Box::new(Expression::FunctionCall(
                        Box::new(Expression::Identifier(str!("$knot.plugin.get"))),
                        vec![
                            Expression::String(str!("ksx")),
                            Expression::String(str!("createElement")),
                            Expression::String(str!("1.0")),
                        ]
                    )),
                    vec![Expression::Identifier(str!("Foo"))]
                )
            );
        }

        #[test]
        fn open_element() {
            assert_eq!(
                Expression::from_component(
                    &ast::shape::Component(ast::Component::open_element(
                        str!("Foo"),
                        vec![
                            (str!("bar"), None),
                            (
                                str!("fizz"),
                                Some(ast::shape::Expression(ast::Expression::Primitive(
                                    ast::Primitive::Nil
                                )))
                            ),
                        ],
                        vec![
                            ast::shape::Component(ast::Component::Text(str!("foo"))),
                            ast::shape::Component(ast::Component::Text(str!("bar"))),
                        ],
                        str!("Foo"),
                    )),
                    &OPTIONS
                ),
                Expression::FunctionCall(
                    Box::new(Expression::FunctionCall(
                        Box::new(Expression::Identifier(str!("$knot.plugin.get"))),
                        vec![
                            Expression::String(str!("ksx")),
                            Expression::String(str!("createElement")),
                            Expression::String(str!("1.0")),
                        ]
                    )),
                    vec![
                        Expression::Identifier(str!("Foo")),
                        Expression::Object(vec![
                            (str!("bar"), Expression::Identifier(str!("bar"))),
                            (str!("fizz"), Expression::Null)
                        ]),
                        Expression::String(str!("foo")),
                        Expression::String(str!("bar")),
                    ]
                )
            );
        }

        #[test]
        fn open_element_no_attributes() {
            assert_eq!(
                Expression::from_component(
                    &ast::shape::Component(ast::Component::open_element(
                        str!("Foo"),
                        vec![],
                        vec![
                            ast::shape::Component(ast::Component::Text(str!("foo"))),
                            ast::shape::Component(ast::Component::Text(str!("bar"))),
                        ],
                        str!("Foo"),
                    )),
                    &OPTIONS
                ),
                Expression::FunctionCall(
                    Box::new(Expression::FunctionCall(
                        Box::new(Expression::Identifier(str!("$knot.plugin.get"))),
                        vec![
                            Expression::String(str!("ksx")),
                            Expression::String(str!("createElement")),
                            Expression::String(str!("1.0")),
                        ]
                    )),
                    vec![
                        Expression::Identifier(str!("Foo")),
                        Expression::Null,
                        Expression::String(str!("foo")),
                        Expression::String(str!("bar")),
                    ]
                )
            );
        }

        #[test]
        fn open_element_no_children() {
            assert_eq!(
                Expression::from_component(
                    &ast::shape::Component(ast::Component::open_element(
                        str!("Foo"),
                        vec![
                            (str!("bar"), None),
                            (
                                str!("fizz"),
                                Some(ast::shape::Expression(ast::Expression::Primitive(
                                    ast::Primitive::Nil
                                )))
                            ),
                        ],
                        vec![],
                        str!("Foo"),
                    )),
                    &OPTIONS
                ),
                Expression::FunctionCall(
                    Box::new(Expression::FunctionCall(
                        Box::new(Expression::Identifier(str!("$knot.plugin.get"))),
                        vec![
                            Expression::String(str!("ksx")),
                            Expression::String(str!("createElement")),
                            Expression::String(str!("1.0")),
                        ]
                    )),
                    vec![
                        Expression::Identifier(str!("Foo")),
                        Expression::Object(vec![
                            (str!("bar"), Expression::Identifier(str!("bar"))),
                            (str!("fizz"), Expression::Null)
                        ]),
                    ]
                )
            );
        }
    }
}