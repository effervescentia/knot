pub mod binary_operation;
pub mod primitive;
pub mod style;

use crate::{ksx, matcher as m, statement, Position, Range};
use combine::{choice, many, parser, position, sep_end_by, Parser, Stream};
use lang::ast::{AstNode, Expression, ExpressionNode, UnaryOperator};

fn primitive<T>() -> impl Parser<T, Output = ExpressionNode<Range, ()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    m::lexeme(primitive::primitive().map(Expression::Primitive))
        .map(|(x, range)| ExpressionNode::raw(x, range))
}

fn group<T, P>(parser: P) -> impl Parser<T, Output = ExpressionNode<Range, ()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
    P: Parser<T, Output = ExpressionNode<Range, ()>>,
{
    m::between(m::symbol('('), m::symbol(')'), parser)
        .map(|(x, range)| ExpressionNode::raw(Expression::Group(Box::new(x)), range))
}

fn identifier<T>() -> impl Parser<T, Output = ExpressionNode<Range, ()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    m::standard_identifier().map(|(x, range)| ExpressionNode::raw(Expression::Identifier(x), range))
}

fn closure<T, P>(parser: impl Fn() -> P) -> impl Parser<T, Output = ExpressionNode<Range, ()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
    P: Parser<T, Output = ExpressionNode<Range, ()>>,
{
    m::between(
        m::symbol('{'),
        m::symbol('}'),
        many::<Vec<_>, _, _>(statement::statement(parser)),
    )
    .map(|(xs, range)| ExpressionNode::raw(Expression::Closure(xs), range))
}

fn unary_operation<T, P>(
    parser: impl Fn() -> P,
) -> impl Parser<T, Output = ExpressionNode<Range, ()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
    P: Parser<T, Output = ExpressionNode<Range, ()>>,
{
    let operation = |c, op| {
        (position(), m::symbol(c), parser()).map(move |(start, _, x)| {
            let range = x.node().range().include(&start);

            ExpressionNode::raw(Expression::UnaryOperation(op, Box::new(x)), range)
        })
    };

    choice((
        operation('!', UnaryOperator::Not),
        operation('+', UnaryOperator::Absolute),
        operation('-', UnaryOperator::Negate),
    ))
    .or(parser())
}

fn dot_access<T, P>(parser: P) -> impl Parser<T, Output = ExpressionNode<Range, ()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
    P: Parser<T, Output = ExpressionNode<Range, ()>>,
{
    m::folding(
        parser,
        m::symbol('.').with(m::standard_identifier()),
        |lhs, (rhs, end)| {
            let range = lhs.node().range() + &end;

            ExpressionNode::raw(Expression::DotAccess(Box::new(lhs), rhs), range)
        },
    )
}

fn function_call<T, P1, P2>(lhs: P1, rhs: P2) -> impl Parser<T, Output = ExpressionNode<Range, ()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
    P1: Parser<T, Output = ExpressionNode<Range, ()>>,
    P2: Parser<T, Output = ExpressionNode<Range, ()>>,
{
    m::folding(
        lhs,
        m::between(
            m::symbol('('),
            m::symbol(')'),
            sep_end_by::<Vec<_>, _, _, _>(rhs, m::symbol(',')),
        ),
        |acc, (args, end)| {
            let range = acc.node().range() + &end;

            ExpressionNode::raw(Expression::FunctionCall(Box::new(acc), args), range)
        },
    )
}

fn ksx<T>() -> impl Parser<T, Output = ExpressionNode<Range, ()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    ksx::ksx().map(|ksx| {
        let range = *ksx.node().range();

        ExpressionNode::raw(Expression::KSX(Box::new(ksx)), range)
    })
}

fn expression_8<T>() -> impl Parser<T, Output = ExpressionNode<Range, ()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    choice((primitive(), style::style(expression), ksx(), identifier()))
}

fn expression_7<T>() -> impl Parser<T, Output = ExpressionNode<Range, ()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    choice((closure(expression), group(expression()), expression_8()))
}

fn expression_6<T>() -> impl Parser<T, Output = ExpressionNode<Range, ()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    dot_access(expression_7())
}

fn expression_5<T>() -> impl Parser<T, Output = ExpressionNode<Range, ()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    function_call(expression_6(), expression())
}

fn expression_4<T>() -> impl Parser<T, Output = ExpressionNode<Range, ()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    unary_operation(expression_5)
}

fn expression_3<T>() -> impl Parser<T, Output = ExpressionNode<Range, ()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    binary_operation::arithmetic(expression_4())
}

fn expression_2<T>() -> impl Parser<T, Output = ExpressionNode<Range, ()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    binary_operation::relational(expression_3())
}

fn expression_1<T>() -> impl Parser<T, Output = ExpressionNode<Range, ()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    binary_operation::comparative(expression_2())
}

fn expression_0<T>() -> impl Parser<T, Output = ExpressionNode<Range, ()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    binary_operation::logical(expression_1())
}

parser! {
    pub fn expression[T]()(T) -> ExpressionNode<Range, ()>
    where
        [T: Stream<Token = char>, T::Position: Position]
    {
        expression_0()
    }
}

pub fn ksx_term<T>() -> impl Parser<T, Output = ExpressionNode<Range, ()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    unary_operation(|| function_call(expression_8(), expression()))
}

#[cfg(test)]
mod tests {
    use crate::{test::fixture as f, Range};
    use combine::{eof, stream::position::Stream, EasyParser, Parser};
    use lang::ast::{
        BinaryOperator, Expression, ExpressionNode, Primitive, Statement, UnaryOperator, KSX,
    };

    fn parse(s: &str) -> crate::Result<ExpressionNode<Range, ()>> {
        super::expression().skip(eof()).easy_parse(Stream::new(s))
    }

    #[test]
    fn primitive() {
        assert_eq!(
            parse("nil").unwrap().0,
            f::n::xr(Expression::Primitive(Primitive::Nil), ((1, 1), (1, 3)))
        );
        assert_eq!(
            parse("true").unwrap().0,
            f::n::xr(
                Expression::Primitive(Primitive::Boolean(true)),
                ((1, 1), (1, 4))
            )
        );
        assert_eq!(
            parse("false").unwrap().0,
            f::n::xr(
                Expression::Primitive(Primitive::Boolean(false)),
                ((1, 1), (1, 5))
            )
        );
        assert_eq!(
            parse("123").unwrap().0,
            f::n::xr(
                Expression::Primitive(Primitive::Integer(123)),
                ((1, 1), (1, 3))
            )
        );
        assert_eq!(
            parse("123.456").unwrap().0,
            f::n::xr(
                Expression::Primitive(Primitive::Float(123.456, 3)),
                ((1, 1), (1, 7))
            )
        );
        assert_eq!(
            parse("\"foo\"").unwrap().0,
            f::n::xr(
                Expression::Primitive(Primitive::String(String::from("foo"))),
                ((1, 1), (1, 5))
            )
        );
    }

    #[test]
    fn identifier() {
        assert_eq!(
            parse("foo").unwrap().0,
            f::n::xr(
                Expression::Identifier(String::from("foo")),
                ((1, 1), (1, 3))
            )
        );
    }

    #[test]
    fn group() {
        assert_eq!(
            parse("(nil)").unwrap().0,
            f::n::xr(
                Expression::Group(Box::new(f::n::xr(
                    Expression::Primitive(Primitive::Nil),
                    ((1, 2), (1, 4))
                ))),
                ((1, 1), (1, 5))
            )
        );
    }

    #[test]
    fn closure() {
        assert_eq!(
            parse("{ nil; nil }").unwrap().0,
            f::n::xr(
                Expression::Closure(vec![
                    f::n::sr(
                        Statement::Expression(f::n::xr(
                            Expression::Primitive(Primitive::Nil),
                            ((1, 3), (1, 5))
                        )),
                        ((1, 3), (1, 5))
                    ),
                    f::n::sr(
                        Statement::Expression(f::n::xr(
                            Expression::Primitive(Primitive::Nil),
                            ((1, 8), (1, 10))
                        )),
                        ((1, 8), (1, 10))
                    )
                ]),
                ((1, 1), (1, 12))
            )
        );
        assert_eq!(
            parse("{}").unwrap().0,
            f::n::xr(Expression::Closure(vec![]), ((1, 1), (1, 2)))
        );
    }

    #[test]
    fn unary_not_operation() {
        assert_eq!(
            parse("!nil").unwrap().0,
            f::n::xr(
                Expression::UnaryOperation(
                    UnaryOperator::Not,
                    Box::new(f::n::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 2), (1, 4))
                    )),
                ),
                ((1, 1), (1, 4))
            )
        );
    }

    #[test]
    fn unary_absolute_operation() {
        assert_eq!(
            parse("+nil").unwrap().0,
            f::n::xr(
                Expression::UnaryOperation(
                    UnaryOperator::Absolute,
                    Box::new(f::n::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 2), (1, 4))
                    )),
                ),
                ((1, 1), (1, 4))
            )
        );
    }

    #[test]
    fn unary_negative_operation() {
        assert_eq!(
            parse("-nil").unwrap().0,
            f::n::xr(
                Expression::UnaryOperation(
                    UnaryOperator::Negate,
                    Box::new(f::n::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 2), (1, 4))
                    )),
                ),
                ((1, 1), (1, 4))
            )
        );
    }

    #[test]
    fn binary_add_operation() {
        assert_eq!(
            parse("nil + nil").unwrap().0,
            f::n::xr(
                Expression::BinaryOperation(
                    BinaryOperator::Add,
                    Box::new(f::n::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 1), (1, 3))
                    )),
                    Box::new(f::n::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 7), (1, 9))
                    )),
                ),
                ((1, 1), (1, 9))
            )
        );
    }

    #[test]
    fn binary_subtract_operation() {
        assert_eq!(
            parse("nil - nil").unwrap().0,
            f::n::xr(
                Expression::BinaryOperation(
                    BinaryOperator::Subtract,
                    Box::new(f::n::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 1), (1, 3))
                    )),
                    Box::new(f::n::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 7), (1, 9))
                    )),
                ),
                ((1, 1), (1, 9))
            )
        );
    }

    #[test]
    fn binary_multiply_operation() {
        assert_eq!(
            parse("nil * nil").unwrap().0,
            f::n::xr(
                Expression::BinaryOperation(
                    BinaryOperator::Multiply,
                    Box::new(f::n::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 1), (1, 3))
                    )),
                    Box::new(f::n::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 7), (1, 9))
                    )),
                ),
                ((1, 1), (1, 9))
            )
        );
    }

    #[test]
    fn binary_divide_operation() {
        assert_eq!(
            parse("nil / nil").unwrap().0,
            f::n::xr(
                Expression::BinaryOperation(
                    BinaryOperator::Divide,
                    Box::new(f::n::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 1), (1, 3))
                    )),
                    Box::new(f::n::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 7), (1, 9))
                    )),
                ),
                ((1, 1), (1, 9))
            )
        );
    }

    #[test]
    fn binary_exponent_operation() {
        assert_eq!(
            parse("nil ^ nil").unwrap().0,
            f::n::xr(
                Expression::BinaryOperation(
                    BinaryOperator::Exponent,
                    Box::new(f::n::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 1), (1, 3))
                    )),
                    Box::new(f::n::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 7), (1, 9))
                    )),
                ),
                ((1, 1), (1, 9))
            )
        );
    }

    #[test]
    fn binary_less_than_operation() {
        assert_eq!(
            parse("nil < nil").unwrap().0,
            f::n::xr(
                Expression::BinaryOperation(
                    BinaryOperator::LessThan,
                    Box::new(f::n::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 1), (1, 3))
                    )),
                    Box::new(f::n::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 7), (1, 9))
                    )),
                ),
                ((1, 1), (1, 9))
            )
        );
    }

    #[test]
    fn binary_less_than_or_equal_operation() {
        assert_eq!(
            parse("nil <= nil").unwrap().0,
            f::n::xr(
                Expression::BinaryOperation(
                    BinaryOperator::LessThanOrEqual,
                    Box::new(f::n::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 1), (1, 3))
                    )),
                    Box::new(f::n::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 8), (1, 10))
                    )),
                ),
                ((1, 1), (1, 10))
            )
        );
    }

    #[test]
    fn binary_greater_than_operation() {
        assert_eq!(
            parse("nil > nil").unwrap().0,
            f::n::xr(
                Expression::BinaryOperation(
                    BinaryOperator::GreaterThan,
                    Box::new(f::n::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 1), (1, 3))
                    )),
                    Box::new(f::n::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 7), (1, 9))
                    )),
                ),
                ((1, 1), (1, 9))
            )
        );
    }

    #[test]
    fn binary_greater_than_or_equal_operation() {
        assert_eq!(
            parse("nil >= nil").unwrap().0,
            f::n::xr(
                Expression::BinaryOperation(
                    BinaryOperator::GreaterThanOrEqual,
                    Box::new(f::n::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 1), (1, 3))
                    )),
                    Box::new(f::n::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 8), (1, 10))
                    )),
                ),
                ((1, 1), (1, 10))
            )
        );
    }

    #[test]
    fn binary_equal_operation() {
        assert_eq!(
            parse("nil == nil").unwrap().0,
            f::n::xr(
                Expression::BinaryOperation(
                    BinaryOperator::Equal,
                    Box::new(f::n::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 1), (1, 3))
                    )),
                    Box::new(f::n::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 8), (1, 10))
                    )),
                ),
                ((1, 1), (1, 10))
            )
        );
    }

    #[test]
    fn binary_unequal_operation() {
        assert_eq!(
            parse("nil != nil").unwrap().0,
            f::n::xr(
                Expression::BinaryOperation(
                    BinaryOperator::NotEqual,
                    Box::new(f::n::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 1), (1, 3))
                    )),
                    Box::new(f::n::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 8), (1, 10))
                    )),
                ),
                ((1, 1), (1, 10))
            )
        );
    }

    #[test]
    fn binary_and_operation() {
        assert_eq!(
            parse("nil && nil").unwrap().0,
            f::n::xr(
                Expression::BinaryOperation(
                    BinaryOperator::And,
                    Box::new(f::n::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 1), (1, 3))
                    )),
                    Box::new(f::n::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 8), (1, 10))
                    )),
                ),
                ((1, 1), (1, 10))
            )
        );
    }

    #[test]
    fn binary_or_operation() {
        assert_eq!(
            parse("nil || nil").unwrap().0,
            f::n::xr(
                Expression::BinaryOperation(
                    BinaryOperator::Or,
                    Box::new(f::n::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 1), (1, 3))
                    )),
                    Box::new(f::n::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 8), (1, 10))
                    )),
                ),
                ((1, 1), (1, 10))
            )
        );
    }

    #[test]
    fn dot_access() {
        assert_eq!(
            parse("nil.foo").unwrap().0,
            f::n::xr(
                Expression::DotAccess(
                    Box::new(f::n::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 1), (1, 3))
                    )),
                    String::from("foo")
                ),
                ((1, 1), (1, 7))
            )
        );
    }

    #[test]
    fn function_call_empty() {
        assert_eq!(
            parse("nil()").unwrap().0,
            f::n::xr(
                Expression::FunctionCall(
                    Box::new(f::n::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 1), (1, 3))
                    )),
                    vec![]
                ),
                ((1, 1), (1, 5))
            )
        );
    }

    #[test]
    fn function_call() {
        assert_eq!(
            parse("nil(nil, nil)").unwrap().0,
            f::n::xr(
                Expression::FunctionCall(
                    Box::new(f::n::xr(
                        Expression::Primitive(Primitive::Nil),
                        ((1, 1), (1, 3))
                    )),
                    vec![
                        f::n::xr(Expression::Primitive(Primitive::Nil), ((1, 5), (1, 7))),
                        f::n::xr(Expression::Primitive(Primitive::Nil), ((1, 10), (1, 12)))
                    ]
                ),
                ((1, 1), (1, 13))
            )
        );
    }

    #[test]
    fn style_empty() {
        assert_eq!(
            parse("style {}").unwrap().0,
            f::n::xr(Expression::Style(vec![]), ((1, 1), (1, 8)))
        );
    }

    #[test]
    fn style() {
        assert_eq!(
            parse("style { foo: nil }").unwrap().0,
            f::n::xr(
                Expression::Style(vec![(
                    String::from("foo"),
                    f::n::xr(Expression::Primitive(Primitive::Nil), ((1, 14), (1, 16)))
                )]),
                ((1, 1), (1, 18))
            )
        );
    }

    #[test]
    fn ksx() {
        assert_eq!(
            parse("<foo />").unwrap().0,
            f::n::xr(
                Expression::KSX(Box::new(f::n::kxr(
                    KSX::ClosedElement(String::from("foo"), vec![]),
                    ((1, 1), (1, 7))
                ))),
                ((1, 1), (1, 7))
            )
        );
    }
}
