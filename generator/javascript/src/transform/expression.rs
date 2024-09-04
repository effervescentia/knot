use crate::{
    ast::{Expression, Statement},
    knot, Options,
};
use kore::invariant;

#[allow(clippy::multiple_inherent_impl)]
impl Expression {
    pub fn from_expression<Resolver>(
        value: &knot::shape::Expression,
        opts: &Options<Resolver>,
    ) -> Self {
        match &value.0 {
            knot::Expression::Primitive(x) => match x {
                knot::Primitive::Nil => Self::Null,

                knot::Primitive::Boolean(x) => Self::Boolean(*x),

                knot::Primitive::Integer(x) => Self::Number(format!("{x}")),

                knot::Primitive::Float(x, precision) => {
                    Self::Number(format!("{x:.0$}", *precision as usize))
                }

                knot::Primitive::String(x) => Self::String(x.clone()),
            },

            knot::Expression::Identifier(x) => Self::Identifier(x.clone()),

            knot::Expression::Group(x) => Self::Group(Box::new(Self::from_expression(x, opts))),

            knot::Expression::Closure(xs) if xs.is_empty() => Self::Null,
            knot::Expression::Closure(xs) => {
                let statements = xs
                    .iter()
                    .enumerate()
                    .flat_map(|(index, x)| {
                        Statement::from_statement(x, index == xs.len() - 1, opts)
                    })
                    .collect::<Vec<_>>();

                Self::Closure(statements)
            }

            knot::Expression::UnaryOperation(op, x) => match op {
                knot::UnaryOperator::Not => {
                    Self::UnaryOperation("!", Box::new(Self::from_expression(x, opts)))
                }

                knot::UnaryOperator::Negate => {
                    Self::UnaryOperation("-", Box::new(Self::from_expression(x, opts)))
                }

                knot::UnaryOperator::Absolute => {
                    Self::call_global("Math.abs", vec![Self::from_expression(x, opts)])
                }
            },

            knot::Expression::BinaryOperation(op, lhs, rhs) => {
                let binary_op = |op| {
                    Self::BinaryOperation(
                        op,
                        Box::new(Self::from_expression(lhs, opts)),
                        Box::new(Self::from_expression(rhs, opts)),
                    )
                };

                match op {
                    knot::BinaryOperator::And => binary_op("&&"),
                    knot::BinaryOperator::Or => binary_op("||"),

                    knot::BinaryOperator::Equal => binary_op("==="),
                    knot::BinaryOperator::NotEqual => binary_op("!=="),

                    knot::BinaryOperator::LessThan => binary_op("<"),
                    knot::BinaryOperator::LessThanOrEqual => binary_op("<="),
                    knot::BinaryOperator::GreaterThan => binary_op(">"),
                    knot::BinaryOperator::GreaterThanOrEqual => binary_op(">="),

                    knot::BinaryOperator::Add => binary_op("+"),
                    knot::BinaryOperator::Subtract => binary_op("-"),
                    knot::BinaryOperator::Multiply => binary_op("*"),
                    knot::BinaryOperator::Divide => binary_op("/"),

                    knot::BinaryOperator::Exponent => Self::call_global(
                        "Math.pow",
                        vec![
                            Self::from_expression(lhs, opts),
                            Self::from_expression(rhs, opts),
                        ],
                    ),
                }
            }

            knot::Expression::PropertyAccess(lhs, rhs) => {
                Self::PropertyAccess(Box::new(Self::from_expression(lhs, opts)), rhs.clone())
            }

            knot::Expression::FunctionCall(x, arguments) => Self::FunctionCall(
                Box::new(Self::from_expression(x, opts)),
                arguments
                    .iter()
                    .map(|x| Self::from_expression(x, opts))
                    .collect(),
            ),

            knot::Expression::Style(xs) => Self::FunctionCall(
                Box::new(Self::plugin("style", "create")),
                vec![Self::Object(
                    xs.iter()
                        .map(|(key, value)| (key.clone(), Self::from_expression(value, opts)))
                        .collect(),
                )],
            ),

            knot::Expression::Component(x) => Self::from_component(x, opts),
        }
    }

    pub fn from_component<Resolver>(
        value: &knot::shape::Component,
        opts: &Options<Resolver>,
    ) -> Self {
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
            knot::Component::Text(x) => Self::String(x.clone()),

            knot::Component::Expression(x) => Self::from_expression(x, opts),

            knot::Component::Fragment(xs) => Self::FunctionCall(
                Box::new(Self::plugin("view", "createFragment")),
                xs.iter().map(|x| Self::from_component(x, opts)).collect(),
            ),

            knot::Component::ClosedElement(tag, attributes) => {
                let name_arg = element_name(tag.clone());

                Self::FunctionCall(
                    Box::new(Self::plugin("view", "createElement")),
                    if attributes.is_empty() {
                        vec![name_arg]
                    } else {
                        vec![name_arg, Self::from_attributes(attributes, opts)]
                    },
                )
            }

            knot::Component::OpenElement {
                start_tag,
                attributes,
                children,
                ..
            } => {
                let name_arg = element_name(start_tag.clone());

                Self::FunctionCall(
                    Box::new(Self::plugin("view", "createElement")),
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

    pub fn from_attributes<Resolver>(
        xs: &[knot::shape::Attribute],
        opts: &Options<Resolver>,
    ) -> Self {
        if xs.is_empty() {
            return Self::Null;
        }

        Self::Object(
            xs.iter()
                .map(|x| {
                    (
                        x.0.name().to_owned(),
                        match &x.0 {
                            knot::Attribute::Explicit(_, x) => Self::from_expression(x, opts),

                            knot::Attribute::Punned(name) => Self::Identifier(name.clone()),
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
        ast::{Expression, Statement},
        test::MOCK_OPTIONS,
    };
    use kore::str;
    use lang::ast;

    mod expression {

        use super::*;

        #[test]
        fn primitive_nil() {
            assert_eq!(
                Expression::from_expression(
                    &ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Nil)),
                    &MOCK_OPTIONS
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
                    &MOCK_OPTIONS
                ),
                Expression::Boolean(true)
            );
            assert_eq!(
                Expression::from_expression(
                    &ast::shape::Expression(ast::Expression::Primitive(ast::Primitive::Boolean(
                        false
                    ))),
                    &MOCK_OPTIONS
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
                    &MOCK_OPTIONS
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
                    &MOCK_OPTIONS
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
                    &MOCK_OPTIONS
                ),
                Expression::String(str!("foo"))
            );
        }

        #[test]
        fn identifier() {
            assert_eq!(
                Expression::from_expression(
                    &ast::shape::Expression(ast::Expression::Identifier(str!("foo"))),
                    &MOCK_OPTIONS
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
                    &MOCK_OPTIONS
                ),
                Expression::Group(Box::new(Expression::Null))
            );
        }

        #[test]
        fn empty_closure() {
            assert_eq!(
                Expression::from_expression(
                    &ast::shape::Expression(ast::Expression::Closure(vec![])),
                    &MOCK_OPTIONS
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
                    &MOCK_OPTIONS
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
                    &MOCK_OPTIONS
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
                    &MOCK_OPTIONS,
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
                    &MOCK_OPTIONS
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
                    &MOCK_OPTIONS,
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
                    &MOCK_OPTIONS
                ),
                Expression::FunctionCall(
                    Box::new(Expression::Identifier(str!("Math.pow"))),
                    vec![Expression::Null, Expression::Null]
                ),
            );
        }

        #[test]
        fn property_access() {
            assert_eq!(
                Expression::from_expression(
                    &ast::shape::Expression(ast::Expression::PropertyAccess(
                        Box::new(ast::shape::Expression(ast::Expression::Primitive(
                            ast::Primitive::Nil
                        ))),
                        str!("foo")
                    )),
                    &MOCK_OPTIONS
                ),
                Expression::PropertyAccess(Box::new(Expression::Null), str!("foo")),
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
                    &MOCK_OPTIONS
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
                    &MOCK_OPTIONS
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
        fn view() {
            assert_eq!(
                Expression::from_expression(
                    &ast::shape::Expression(ast::Expression::Component(Box::new(
                        ast::shape::Component(ast::Component::ClosedElement(str!("Foo"), vec![]))
                    ))),
                    &MOCK_OPTIONS
                ),
                Expression::FunctionCall(
                    Box::new(Expression::FunctionCall(
                        Box::new(Expression::Identifier(str!("$knot.plugin.get"))),
                        vec![
                            Expression::String(str!("view")),
                            Expression::String(str!("createElement")),
                            Expression::String(str!("1.0")),
                        ]
                    )),
                    vec![Expression::Identifier(str!("Foo"))]
                ),
            );
        }
    }

    mod view {
        use super::*;

        #[test]
        fn text() {
            assert_eq!(
                Expression::from_component(
                    &ast::shape::Component(ast::Component::Text(str!("foo"))),
                    &MOCK_OPTIONS
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
                    &MOCK_OPTIONS
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
                    &MOCK_OPTIONS
                ),
                Expression::FunctionCall(
                    Box::new(Expression::FunctionCall(
                        Box::new(Expression::Identifier(str!("$knot.plugin.get"))),
                        vec![
                            Expression::String(str!("view")),
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
                    &MOCK_OPTIONS
                ),
                Expression::FunctionCall(
                    Box::new(Expression::FunctionCall(
                        Box::new(Expression::Identifier(str!("$knot.plugin.get"))),
                        vec![
                            Expression::String(str!("view")),
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
                            ast::shape::Attribute(ast::Attribute::Punned(str!("bar"))),
                            ast::shape::Attribute(ast::Attribute::Explicit(
                                str!("fizz"),
                                ast::shape::Expression(ast::Expression::Primitive(
                                    ast::Primitive::Nil
                                ))
                            )),
                        ]
                    )),
                    &MOCK_OPTIONS
                ),
                Expression::FunctionCall(
                    Box::new(Expression::FunctionCall(
                        Box::new(Expression::Identifier(str!("$knot.plugin.get"))),
                        vec![
                            Expression::String(str!("view")),
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
                    &MOCK_OPTIONS
                ),
                Expression::FunctionCall(
                    Box::new(Expression::FunctionCall(
                        Box::new(Expression::Identifier(str!("$knot.plugin.get"))),
                        vec![
                            Expression::String(str!("view")),
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
                            ast::shape::Attribute(ast::Attribute::Punned(str!("bar"))),
                            ast::shape::Attribute(ast::Attribute::Explicit(
                                str!("fizz"),
                                ast::shape::Expression(ast::Expression::Primitive(
                                    ast::Primitive::Nil
                                ))
                            )),
                        ],
                        vec![
                            ast::shape::Component(ast::Component::Text(str!("foo"))),
                            ast::shape::Component(ast::Component::Text(str!("bar"))),
                        ],
                        str!("Foo"),
                    )),
                    &MOCK_OPTIONS
                ),
                Expression::FunctionCall(
                    Box::new(Expression::FunctionCall(
                        Box::new(Expression::Identifier(str!("$knot.plugin.get"))),
                        vec![
                            Expression::String(str!("view")),
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
                    &MOCK_OPTIONS
                ),
                Expression::FunctionCall(
                    Box::new(Expression::FunctionCall(
                        Box::new(Expression::Identifier(str!("$knot.plugin.get"))),
                        vec![
                            Expression::String(str!("view")),
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
                            ast::shape::Attribute(ast::Attribute::Punned(str!("bar"))),
                            ast::shape::Attribute(ast::Attribute::Explicit(
                                str!("fizz"),
                                ast::shape::Expression(ast::Expression::Primitive(
                                    ast::Primitive::Nil
                                ))
                            )),
                        ],
                        vec![],
                        str!("Foo"),
                    )),
                    &MOCK_OPTIONS
                ),
                Expression::FunctionCall(
                    Box::new(Expression::FunctionCall(
                        Box::new(Expression::Identifier(str!("$knot.plugin.get"))),
                        vec![
                            Expression::String(str!("view")),
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
