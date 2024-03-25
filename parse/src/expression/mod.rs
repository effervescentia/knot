pub mod binary_operation;
pub mod primitive;
pub mod style;

use crate::{
    ast, component,
    matcher::{self as m, Position},
    statement,
};
use combine::{choice, many, parser, position, sep_end_by, Parser, Stream};

fn primitive<T>() -> impl Parser<T, Output = ast::raw::Expression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    // TODO: refactor to only map one time
    m::lexeme(primitive::primitive().map(ast::Expression::Primitive))
        .map(|(x, range)| ast::raw::Expression::new(x, range))
}

fn group<T, P>(parser: P) -> impl Parser<T, Output = ast::raw::Expression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
    P: Parser<T, Output = ast::raw::Expression>,
{
    m::between(m::symbol('('), m::symbol(')'), parser)
        .map(|(x, range)| ast::raw::Expression::new(ast::Expression::Group(Box::new(x)), range))
}

fn identifier<T>() -> impl Parser<T, Output = ast::raw::Expression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    m::standard_identifier()
        .map(|(x, range)| ast::raw::Expression::new(ast::Expression::Identifier(x), range))
}

fn closure<T, P>(parser: impl Fn() -> P) -> impl Parser<T, Output = ast::raw::Expression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
    P: Parser<T, Output = ast::raw::Expression>,
{
    m::between(
        m::symbol('{'),
        m::symbol('}'),
        many::<Vec<_>, _, _>(statement::statement(parser)),
    )
    .map(|(xs, range)| ast::raw::Expression::new(ast::Expression::Closure(xs), range))
}

fn unary_operation<T, P>(parser: impl Fn() -> P) -> impl Parser<T, Output = ast::raw::Expression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
    P: Parser<T, Output = ast::raw::Expression>,
{
    let operation = |c, op| {
        (position(), m::symbol(c), parser()).map(move |(start, _, x)| {
            let range = x.0.range().extend(&start.to_point());

            ast::raw::Expression::new(ast::Expression::UnaryOperation(op, Box::new(x)), range)
        })
    };

    choice((
        operation('!', ast::UnaryOperator::Not),
        operation('+', ast::UnaryOperator::Absolute),
        operation('-', ast::UnaryOperator::Negate),
    ))
    .or(parser())
}

fn property_access<T, P>(parser: P) -> impl Parser<T, Output = ast::raw::Expression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
    P: Parser<T, Output = ast::raw::Expression>,
{
    m::folding(
        parser,
        m::symbol('.').with(m::standard_identifier()),
        |lhs, (rhs, end)| {
            let range = lhs.0.range() + &end;

            ast::raw::Expression::new(ast::Expression::PropertyAccess(Box::new(lhs), rhs), range)
        },
    )
}

fn function_call<T, P1, P2>(lhs: P1, rhs: P2) -> impl Parser<T, Output = ast::raw::Expression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
    P1: Parser<T, Output = ast::raw::Expression>,
    P2: Parser<T, Output = ast::raw::Expression>,
{
    m::folding(
        lhs,
        m::between(
            m::symbol('('),
            m::symbol(')'),
            sep_end_by::<Vec<_>, _, _, _>(rhs, m::symbol(',')),
        ),
        |acc, (args, end)| {
            let range = acc.0.range() + &end;

            ast::raw::Expression::new(ast::Expression::FunctionCall(Box::new(acc), args), range)
        },
    )
}

fn component<T>() -> impl Parser<T, Output = ast::raw::Expression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    component::component().map(|ksx| {
        let range = *ksx.0.range();

        ast::raw::Expression::new(ast::Expression::Component(Box::new(ksx)), range)
    })
}

fn expression_8<T>() -> impl Parser<T, Output = ast::raw::Expression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    choice((
        primitive(),
        style::style(expression),
        component(),
        identifier(),
    ))
}

fn expression_7<T>() -> impl Parser<T, Output = ast::raw::Expression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    choice((closure(expression), group(expression()), expression_8()))
}

fn expression_6<T>() -> impl Parser<T, Output = ast::raw::Expression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    property_access(expression_7())
}

fn expression_5<T>() -> impl Parser<T, Output = ast::raw::Expression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    function_call(expression_6(), expression())
}

fn expression_4<T>() -> impl Parser<T, Output = ast::raw::Expression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    unary_operation(expression_5)
}

fn expression_3<T>() -> impl Parser<T, Output = ast::raw::Expression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    binary_operation::arithmetic(expression_4())
}

fn expression_2<T>() -> impl Parser<T, Output = ast::raw::Expression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    binary_operation::relational(expression_3())
}

fn expression_1<T>() -> impl Parser<T, Output = ast::raw::Expression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    binary_operation::comparative(expression_2())
}

fn expression_0<T>() -> impl Parser<T, Output = ast::raw::Expression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    binary_operation::logical(expression_1())
}

parser! {
    pub fn expression[T]()(T) -> ast::raw::Expression
    where
        [T: Stream<Token = char>, T::Position: m::Position]
    {
        expression_0()
    }
}

pub fn component_term<T>() -> impl Parser<T, Output = ast::raw::Expression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    unary_operation(|| function_call(expression_8(), expression()))
}

#[cfg(test)]
mod tests {
    use crate::ast;
    use combine::{eof, stream::position::Stream, EasyParser, Parser};
    use kore::str;
    use lang::Range;

    fn parse(s: &str) -> crate::Result<ast::raw::Expression> {
        super::expression().skip(eof()).easy_parse(Stream::new(s))
    }

    #[test]
    fn primitive() {
        assert_eq!(
            parse("nil").unwrap().0,
            ast::raw::Expression::new(
                ast::Expression::Primitive(ast::Primitive::Nil),
                Range::new((1, 1), (1, 3))
            )
        );
        assert_eq!(
            parse("true").unwrap().0,
            ast::raw::Expression::new(
                ast::Expression::Primitive(ast::Primitive::Boolean(true)),
                Range::new((1, 1), (1, 4))
            )
        );
        assert_eq!(
            parse("false").unwrap().0,
            ast::raw::Expression::new(
                ast::Expression::Primitive(ast::Primitive::Boolean(false)),
                Range::new((1, 1), (1, 5))
            )
        );
        assert_eq!(
            parse("123").unwrap().0,
            ast::raw::Expression::new(
                ast::Expression::Primitive(ast::Primitive::Integer(123)),
                Range::new((1, 1), (1, 3))
            )
        );
        assert_eq!(
            parse("123.456").unwrap().0,
            ast::raw::Expression::new(
                ast::Expression::Primitive(ast::Primitive::Float(123.456, 3)),
                Range::new((1, 1), (1, 7))
            )
        );
        assert_eq!(
            parse("\"foo\"").unwrap().0,
            ast::raw::Expression::new(
                ast::Expression::Primitive(ast::Primitive::String(str!("foo"))),
                Range::new((1, 1), (1, 5))
            )
        );
    }

    #[test]
    fn identifier() {
        assert_eq!(
            parse("foo").unwrap().0,
            ast::raw::Expression::new(
                ast::Expression::Identifier(str!("foo")),
                Range::new((1, 1), (1, 3))
            )
        );
    }

    #[test]
    fn group() {
        assert_eq!(
            parse("(nil)").unwrap().0,
            ast::raw::Expression::new(
                ast::Expression::Group(Box::new(ast::raw::Expression::new(
                    ast::Expression::Primitive(ast::Primitive::Nil),
                    Range::new((1, 2), (1, 4))
                ))),
                Range::new((1, 1), (1, 5))
            )
        );
    }

    #[test]
    fn closure() {
        assert_eq!(
            parse("{ nil; nil }").unwrap().0,
            ast::raw::Expression::new(
                ast::Expression::Closure(vec![
                    ast::raw::Statement::new(
                        ast::Statement::Expression(ast::raw::Expression::new(
                            ast::Expression::Primitive(ast::Primitive::Nil),
                            Range::new((1, 3), (1, 5))
                        )),
                        Range::new((1, 3), (1, 5))
                    ),
                    ast::raw::Statement::new(
                        ast::Statement::Expression(ast::raw::Expression::new(
                            ast::Expression::Primitive(ast::Primitive::Nil),
                            Range::new((1, 8), (1, 10))
                        )),
                        Range::new((1, 8), (1, 10))
                    )
                ]),
                Range::new((1, 1), (1, 12))
            )
        );
        assert_eq!(
            parse("{}").unwrap().0,
            ast::raw::Expression::new(ast::Expression::Closure(vec![]), Range::new((1, 1), (1, 2)))
        );
    }

    #[test]
    fn unary_not_operation() {
        assert_eq!(
            parse("!nil").unwrap().0,
            ast::raw::Expression::new(
                ast::Expression::UnaryOperation(
                    ast::UnaryOperator::Not,
                    Box::new(ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 2), (1, 4))
                    )),
                ),
                Range::new((1, 1), (1, 4))
            )
        );
    }

    #[test]
    fn unary_absolute_operation() {
        assert_eq!(
            parse("+nil").unwrap().0,
            ast::raw::Expression::new(
                ast::Expression::UnaryOperation(
                    ast::UnaryOperator::Absolute,
                    Box::new(ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 2), (1, 4))
                    )),
                ),
                Range::new((1, 1), (1, 4))
            )
        );
    }

    #[test]
    fn unary_negative_operation() {
        assert_eq!(
            parse("-nil").unwrap().0,
            ast::raw::Expression::new(
                ast::Expression::UnaryOperation(
                    ast::UnaryOperator::Negate,
                    Box::new(ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 2), (1, 4))
                    )),
                ),
                Range::new((1, 1), (1, 4))
            )
        );
    }

    #[test]
    fn binary_add_operation() {
        assert_eq!(
            parse("nil + nil").unwrap().0,
            ast::raw::Expression::new(
                ast::Expression::BinaryOperation(
                    ast::BinaryOperator::Add,
                    Box::new(ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 1), (1, 3))
                    )),
                    Box::new(ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 7), (1, 9))
                    )),
                ),
                Range::new((1, 1), (1, 9))
            )
        );
    }

    #[test]
    fn binary_subtract_operation() {
        assert_eq!(
            parse("nil - nil").unwrap().0,
            ast::raw::Expression::new(
                ast::Expression::BinaryOperation(
                    ast::BinaryOperator::Subtract,
                    Box::new(ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 1), (1, 3))
                    )),
                    Box::new(ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 7), (1, 9))
                    )),
                ),
                Range::new((1, 1), (1, 9))
            )
        );
    }

    #[test]
    fn binary_multiply_operation() {
        assert_eq!(
            parse("nil * nil").unwrap().0,
            ast::raw::Expression::new(
                ast::Expression::BinaryOperation(
                    ast::BinaryOperator::Multiply,
                    Box::new(ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 1), (1, 3))
                    )),
                    Box::new(ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 7), (1, 9))
                    )),
                ),
                Range::new((1, 1), (1, 9))
            )
        );
    }

    #[test]
    fn binary_divide_operation() {
        assert_eq!(
            parse("nil / nil").unwrap().0,
            ast::raw::Expression::new(
                ast::Expression::BinaryOperation(
                    ast::BinaryOperator::Divide,
                    Box::new(ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 1), (1, 3))
                    )),
                    Box::new(ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 7), (1, 9))
                    )),
                ),
                Range::new((1, 1), (1, 9))
            )
        );
    }

    #[test]
    fn binary_exponent_operation() {
        assert_eq!(
            parse("nil ^ nil").unwrap().0,
            ast::raw::Expression::new(
                ast::Expression::BinaryOperation(
                    ast::BinaryOperator::Exponent,
                    Box::new(ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 1), (1, 3))
                    )),
                    Box::new(ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 7), (1, 9))
                    )),
                ),
                Range::new((1, 1), (1, 9))
            )
        );
    }

    #[test]
    fn binary_less_than_operation() {
        assert_eq!(
            parse("nil < nil").unwrap().0,
            ast::raw::Expression::new(
                ast::Expression::BinaryOperation(
                    ast::BinaryOperator::LessThan,
                    Box::new(ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 1), (1, 3))
                    )),
                    Box::new(ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 7), (1, 9))
                    )),
                ),
                Range::new((1, 1), (1, 9))
            )
        );
    }

    #[test]
    fn binary_less_than_or_equal_operation() {
        assert_eq!(
            parse("nil <= nil").unwrap().0,
            ast::raw::Expression::new(
                ast::Expression::BinaryOperation(
                    ast::BinaryOperator::LessThanOrEqual,
                    Box::new(ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 1), (1, 3))
                    )),
                    Box::new(ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 8), (1, 10))
                    )),
                ),
                Range::new((1, 1), (1, 10))
            )
        );
    }

    #[test]
    fn binary_greater_than_operation() {
        assert_eq!(
            parse("nil > nil").unwrap().0,
            ast::raw::Expression::new(
                ast::Expression::BinaryOperation(
                    ast::BinaryOperator::GreaterThan,
                    Box::new(ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 1), (1, 3))
                    )),
                    Box::new(ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 7), (1, 9))
                    )),
                ),
                Range::new((1, 1), (1, 9))
            )
        );
    }

    #[test]
    fn binary_greater_than_or_equal_operation() {
        assert_eq!(
            parse("nil >= nil").unwrap().0,
            ast::raw::Expression::new(
                ast::Expression::BinaryOperation(
                    ast::BinaryOperator::GreaterThanOrEqual,
                    Box::new(ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 1), (1, 3))
                    )),
                    Box::new(ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 8), (1, 10))
                    )),
                ),
                Range::new((1, 1), (1, 10))
            )
        );
    }

    #[test]
    fn binary_equal_operation() {
        assert_eq!(
            parse("nil == nil").unwrap().0,
            ast::raw::Expression::new(
                ast::Expression::BinaryOperation(
                    ast::BinaryOperator::Equal,
                    Box::new(ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 1), (1, 3))
                    )),
                    Box::new(ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 8), (1, 10))
                    )),
                ),
                Range::new((1, 1), (1, 10))
            )
        );
    }

    #[test]
    fn binary_unequal_operation() {
        assert_eq!(
            parse("nil != nil").unwrap().0,
            ast::raw::Expression::new(
                ast::Expression::BinaryOperation(
                    ast::BinaryOperator::NotEqual,
                    Box::new(ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 1), (1, 3))
                    )),
                    Box::new(ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 8), (1, 10))
                    )),
                ),
                Range::new((1, 1), (1, 10))
            )
        );
    }

    #[test]
    fn binary_and_operation() {
        assert_eq!(
            parse("nil && nil").unwrap().0,
            ast::raw::Expression::new(
                ast::Expression::BinaryOperation(
                    ast::BinaryOperator::And,
                    Box::new(ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 1), (1, 3))
                    )),
                    Box::new(ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 8), (1, 10))
                    )),
                ),
                Range::new((1, 1), (1, 10))
            )
        );
    }

    #[test]
    fn binary_or_operation() {
        assert_eq!(
            parse("nil || nil").unwrap().0,
            ast::raw::Expression::new(
                ast::Expression::BinaryOperation(
                    ast::BinaryOperator::Or,
                    Box::new(ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 1), (1, 3))
                    )),
                    Box::new(ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 8), (1, 10))
                    )),
                ),
                Range::new((1, 1), (1, 10))
            )
        );
    }

    #[test]
    fn dot_access() {
        assert_eq!(
            parse("nil.foo").unwrap().0,
            ast::raw::Expression::new(
                ast::Expression::PropertyAccess(
                    Box::new(ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 1), (1, 3))
                    )),
                    str!("foo")
                ),
                Range::new((1, 1), (1, 7))
            )
        );
    }

    #[test]
    fn function_call_empty() {
        assert_eq!(
            parse("nil()").unwrap().0,
            ast::raw::Expression::new(
                ast::Expression::FunctionCall(
                    Box::new(ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 1), (1, 3))
                    )),
                    vec![]
                ),
                Range::new((1, 1), (1, 5))
            )
        );
    }

    #[test]
    fn function_call() {
        assert_eq!(
            parse("nil(nil, nil)").unwrap().0,
            ast::raw::Expression::new(
                ast::Expression::FunctionCall(
                    Box::new(ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 1), (1, 3))
                    )),
                    vec![
                        ast::raw::Expression::new(
                            ast::Expression::Primitive(ast::Primitive::Nil),
                            Range::new((1, 5), (1, 7))
                        ),
                        ast::raw::Expression::new(
                            ast::Expression::Primitive(ast::Primitive::Nil),
                            Range::new((1, 10), (1, 12))
                        )
                    ]
                ),
                Range::new((1, 1), (1, 13))
            )
        );
    }

    #[test]
    fn style_empty() {
        assert_eq!(
            parse("style {}").unwrap().0,
            ast::raw::Expression::new(ast::Expression::Style(vec![]), Range::new((1, 1), (1, 8)))
        );
    }

    #[test]
    fn style() {
        assert_eq!(
            parse("style { foo: nil }").unwrap().0,
            ast::raw::Expression::new(
                ast::Expression::Style(vec![(
                    str!("foo"),
                    ast::raw::Expression::new(
                        ast::Expression::Primitive(ast::Primitive::Nil),
                        Range::new((1, 14), (1, 16))
                    )
                )]),
                Range::new((1, 1), (1, 18))
            )
        );
    }

    #[test]
    fn ksx() {
        assert_eq!(
            parse("<foo />").unwrap().0,
            ast::raw::Expression::new(
                ast::Expression::Component(Box::new(ast::raw::Component::new(
                    ast::Component::ClosedElement(str!("foo"), vec![]),
                    Range::new((1, 1), (1, 7))
                ))),
                Range::new((1, 1), (1, 7))
            )
        );
    }
}
