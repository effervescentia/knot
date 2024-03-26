use crate::ast;
use kore::str;
use lang::Range;

type Offset = (usize, usize);

pub mod function {
    use super::*;

    pub const SOURCE: &str =
        "func my_func(first, second: integer, third = true): boolean -> first > second || third;";

    pub fn raw(offset: Offset) -> ast::raw::Declaration {
        ast::raw::Declaration::raw(
            ast::Declaration::function(
                ast::Storage::public(ast::raw::Binding::new(
                    ast::Binding(str!("my_func")),
                    Range::new((1, 6), (1, 12)).offset(offset),
                )),
                vec![
                    ast::raw::Parameter::raw(
                        ast::Parameter::new(
                            ast::raw::Binding::new(
                                ast::Binding(str!("first")),
                                Range::new((1, 14), (1, 18)).offset(offset),
                            ),
                            None,
                            None,
                        ),
                        Range::new((1, 14), (1, 18)).offset(offset),
                    ),
                    ast::raw::Parameter::raw(
                        ast::Parameter::new(
                            ast::raw::Binding::new(
                                ast::Binding(str!("second")),
                                Range::new((1, 21), (1, 26)).offset(offset),
                            ),
                            Some(ast::raw::TypeExpression::raw(
                                ast::TypeExpression::Primitive(ast::TypePrimitive::Integer),
                                Range::new((1, 29), (1, 35)).offset(offset),
                            )),
                            None,
                        ),
                        Range::new((1, 21), (1, 35)).offset(offset),
                    ),
                    ast::raw::Parameter::raw(
                        ast::Parameter::new(
                            ast::raw::Binding::new(
                                ast::Binding(str!("third")),
                                Range::new((1, 38), (1, 42)).offset(offset),
                            ),
                            None,
                            Some(ast::raw::Expression::raw(
                                ast::Expression::Primitive(ast::Primitive::Boolean(true)),
                                Range::new((1, 46), (1, 49)).offset(offset),
                            )),
                        ),
                        Range::new((1, 38), (1, 49)).offset(offset),
                    ),
                ],
                Some(ast::raw::TypeExpression::raw(
                    ast::TypeExpression::Primitive(ast::TypePrimitive::Boolean),
                    Range::new((1, 53), (1, 59)).offset(offset),
                )),
                ast::raw::Expression::raw(
                    ast::Expression::BinaryOperation(
                        ast::BinaryOperator::Or,
                        Box::new(ast::raw::Expression::raw(
                            ast::Expression::BinaryOperation(
                                ast::BinaryOperator::GreaterThan,
                                Box::new(ast::raw::Expression::raw(
                                    ast::Expression::Identifier(str!("first")),
                                    Range::new((1, 64), (1, 68)).offset(offset),
                                )),
                                Box::new(ast::raw::Expression::raw(
                                    ast::Expression::Identifier(str!("second")),
                                    Range::new((1, 72), (1, 77)).offset(offset),
                                )),
                            ),
                            Range::new((1, 64), (1, 77)).offset(offset),
                        )),
                        Box::new(ast::raw::Expression::raw(
                            ast::Expression::Identifier(str!("third")),
                            Range::new((1, 82), (1, 86)).offset(offset),
                        )),
                    ),
                    Range::new((1, 64), (1, 86)).offset(offset),
                ),
            ),
            Range::new((1, 1), (1, 86)).offset(offset),
        )
    }
}

pub mod view {
    use super::*;

    pub const SOURCE: &str = "view MyView(inner: element = <div />) -> {
  let value = 123 + 45.67;

  <>
    <h1>Welcome!</h1>
    <main>{value}: {inner}</main>
  </>;
};";

    pub fn raw(offset: Offset) -> ast::raw::Declaration {
        ast::raw::Declaration::raw(
            ast::Declaration::view(
                ast::Storage::public(ast::raw::Binding::new(
                    ast::Binding(str!("MyView")),
                    Range::new((1, 6), (1, 11)).offset(offset),
                )),
                vec![ast::raw::Parameter::raw(
                    ast::Parameter::new(
                        ast::raw::Binding::new(
                            ast::Binding(str!("inner")),
                            Range::new((1, 13), (1, 17)).offset(offset),
                        ),
                        Some(ast::raw::TypeExpression::raw(
                            ast::TypeExpression::Primitive(ast::TypePrimitive::Element),
                            Range::new((1, 20), (1, 26)).offset(offset),
                        )),
                        Some(ast::raw::Expression::raw(
                            ast::Expression::Component(Box::new(ast::raw::Component::raw(
                                ast::Component::ClosedElement(str!("div"), vec![]),
                                Range::new((1, 30), (1, 36)).offset(offset),
                            ))),
                            Range::new((1, 30), (1, 36)).offset(offset),
                        )),
                    ),
                    Range::new((1, 13), (1, 36)).offset(offset),
                )],
                ast::raw::Expression::raw(
                    ast::Expression::Closure(vec![
                        ast::raw::Statement::raw(
                            ast::Statement::Variable(
                                str!("value"),
                                ast::raw::Expression::raw(
                                    ast::Expression::BinaryOperation(
                                        ast::BinaryOperator::Add,
                                        Box::new(ast::raw::Expression::raw(
                                            ast::Expression::Primitive(ast::Primitive::Integer(
                                                123,
                                            )),
                                            Range::new((2, 15), (2, 17)).offset(offset),
                                        )),
                                        Box::new(ast::raw::Expression::raw(
                                            ast::Expression::Primitive(ast::Primitive::Float(
                                                45.67, 2,
                                            )),
                                            Range::new((2, 21), (2, 25)).offset(offset),
                                        )),
                                    ),
                                    Range::new((2, 15), (2, 25)).offset(offset),
                                ),
                            ),
                            Range::new((2, 3), (2, 25)).offset(offset),
                        ),
                        ast::raw::Statement::raw(
                            ast::Statement::Expression(ast::raw::Expression::raw(
                                ast::Expression::Component(Box::new(ast::raw::Component::raw(
                                    ast::Component::Fragment(vec![
                                        ast::raw::Component::raw(
                                            ast::Component::open_element(
                                                str!("h1"),
                                                vec![],
                                                vec![ast::raw::Component::raw(
                                                    ast::Component::Text(str!("Welcome!")),
                                                    Range::new((5, 9), (5, 16)).offset(offset),
                                                )],
                                                str!("h1"),
                                            ),
                                            Range::new((5, 5), (5, 21)).offset(offset),
                                        ),
                                        ast::raw::Component::raw(
                                            ast::Component::open_element(
                                                str!("main"),
                                                vec![],
                                                vec![
                                                    ast::raw::Component::raw(
                                                        ast::Component::Expression(
                                                            ast::raw::Expression::raw(
                                                                ast::Expression::Identifier(str!(
                                                                    "value"
                                                                )),
                                                                Range::new((6, 12), (6, 16))
                                                                    .offset(offset),
                                                            ),
                                                        ),
                                                        Range::new((6, 11), (6, 17)).offset(offset),
                                                    ),
                                                    ast::raw::Component::raw(
                                                        ast::Component::Text(str!(": ")),
                                                        Range::new((6, 18), (6, 19)).offset(offset),
                                                    ),
                                                    ast::raw::Component::raw(
                                                        ast::Component::Expression(
                                                            ast::raw::Expression::raw(
                                                                ast::Expression::Identifier(str!(
                                                                    "inner"
                                                                )),
                                                                Range::new((6, 21), (6, 25))
                                                                    .offset(offset),
                                                            ),
                                                        ),
                                                        Range::new((6, 20), (6, 26)).offset(offset),
                                                    ),
                                                ],
                                                str!("main"),
                                            ),
                                            Range::new((6, 5), (6, 33)).offset(offset),
                                        ),
                                    ]),
                                    Range::new((4, 3), (7, 5)).offset(offset),
                                ))),
                                Range::new((4, 3), (7, 5)).offset(offset),
                            )),
                            Range::new((4, 3), (7, 5)).offset(offset),
                        ),
                    ]),
                    Range::new((1, 42), (8, 1)).offset(offset),
                ),
            ),
            Range::new((1, 1), (8, 1)).offset(offset),
        )
    }
}
