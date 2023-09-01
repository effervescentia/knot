pub mod binary_operation;
pub mod ksx;
pub mod primitive;
pub mod style;
use crate::{
    matcher as m,
    range::Range,
    statement::{self, Statement},
};
use binary_operation::BinaryOperator;
use combine::{attempt, choice, many, parser, position, sep_end_by, Parser, Stream};
use ksx::KSXRaw;
use primitive::Primitive;
use std::fmt::Debug;

#[derive(Clone, Copy, Debug, PartialEq)]
pub enum UnaryOperator {
    Not,
    Absolute,
    Negate,
}

#[derive(Debug, PartialEq)]
pub enum Expression<E, K> {
    Primitive(Primitive),
    Identifier(String),
    Group(Box<E>),
    Closure(Vec<Statement<E>>),
    UnaryOperation(UnaryOperator, Box<E>),
    BinaryOperation(BinaryOperator, Box<E>, Box<E>),
    DotAccess(Box<E>, String),
    FunctionCall(Box<E>, Vec<E>),
    Style(Vec<(String, E)>),
    KSX(Box<K>),
}

#[derive(Debug, PartialEq)]
pub struct ExpressionRaw<T>(pub Expression<ExpressionRaw<T>, KSXRaw<T>>, pub Range<T>)
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug;

impl<T> ExpressionRaw<T>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
{
    pub fn get_range(&self) -> &Range<T> {
        match self {
            ExpressionRaw(_, range) => range,
        }
    }
}

fn primitive<T>() -> impl Parser<T, Output = ExpressionRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
{
    m::lexeme(primitive::primitive().map(Expression::Primitive))
        .map(|(x, range)| ExpressionRaw(x, range))
}

fn group<T, P>(parser: P) -> impl Parser<T, Output = ExpressionRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
    P: Parser<T, Output = ExpressionRaw<T>>,
{
    m::between(m::symbol('('), m::symbol(')'), parser)
        .map(|(x, range)| ExpressionRaw(Expression::Group(Box::new(x)), range))
}

fn identifier<T>() -> impl Parser<T, Output = ExpressionRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
{
    m::standard_identifier().map(|(x, range)| ExpressionRaw(Expression::Identifier(x), range))
}

fn closure<T, P>(parser: impl Fn() -> P) -> impl Parser<T, Output = ExpressionRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
    P: Parser<T, Output = ExpressionRaw<T>>,
{
    m::between(
        m::symbol('{'),
        m::symbol('}'),
        many::<Vec<_>, _, _>(statement::statement(parser)),
    )
    .map(|(xs, range)| ExpressionRaw(Expression::Closure(xs), range))
}

fn unary_operation<T, P>(parser: impl Fn() -> P) -> impl Parser<T, Output = ExpressionRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
    P: Parser<T, Output = ExpressionRaw<T>>,
{
    let operation = |c, op| {
        (position(), m::symbol(c), parser()).map(move |(start, _, x)| {
            let range = x.get_range().include(start);
            ExpressionRaw(Expression::UnaryOperation(op, Box::new(x)), range)
        })
    };

    choice((
        operation('!', UnaryOperator::Not),
        operation('+', UnaryOperator::Absolute),
        operation('-', UnaryOperator::Negate),
    ))
    .or(parser())
}

fn dot_access<T, P>(parser: P) -> impl Parser<T, Output = ExpressionRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
    P: Parser<T, Output = ExpressionRaw<T>>,
{
    m::folding(
        parser,
        m::symbol('.').with(m::standard_identifier()),
        |lhs, (rhs, end)| {
            let range = lhs.get_range().concat(&end);
            ExpressionRaw(Expression::DotAccess(Box::new(lhs), rhs), range)
        },
    )
}

fn function_call<T, P1, P2>(lhs: P1, rhs: P2) -> impl Parser<T, Output = ExpressionRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
    P1: Parser<T, Output = ExpressionRaw<T>>,
    P2: Parser<T, Output = ExpressionRaw<T>>,
{
    m::folding(
        lhs,
        m::between(
            m::symbol('('),
            m::symbol(')'),
            sep_end_by::<Vec<_>, _, _, _>(rhs, m::symbol(',')),
        ),
        |acc, (args, end)| {
            let range = acc.get_range().concat(&end);
            ExpressionRaw(Expression::FunctionCall(Box::new(acc), args), range)
        },
    )
}

fn ksx<T>() -> impl Parser<T, Output = ExpressionRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
{
    ksx::ksx().map(|ksx| {
        let range = ksx.get_range().clone();
        ExpressionRaw(Expression::KSX(Box::new(ksx)), range)
    })
}

fn expression_8<T>() -> impl Parser<T, Output = ExpressionRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
{
    choice((
        attempt(primitive()),
        style::style(expression),
        ksx(),
        identifier(),
    ))
}

fn expression_7<T>() -> impl Parser<T, Output = ExpressionRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
{
    choice((closure(expression), group(expression()), expression_8()))
}

fn expression_6<T>() -> impl Parser<T, Output = ExpressionRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
{
    dot_access(expression_7())
}

fn expression_5<T>() -> impl Parser<T, Output = ExpressionRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
{
    function_call(expression_6(), expression())
}

fn expression_4<T>() -> impl Parser<T, Output = ExpressionRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
{
    unary_operation(expression_5)
}

fn expression_3<T>() -> impl Parser<T, Output = ExpressionRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
{
    binary_operation::arithmetic(expression_4())
}

fn expression_2<T>() -> impl Parser<T, Output = ExpressionRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
{
    binary_operation::relational(expression_3())
}

fn expression_1<T>() -> impl Parser<T, Output = ExpressionRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
{
    binary_operation::comparative(expression_2())
}

fn expression_0<T>() -> impl Parser<T, Output = ExpressionRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
{
    binary_operation::logical(expression_1())
}

parser! {
    pub fn expression[T]()(T) -> ExpressionRaw<T>
    where
        [T: Stream<Token = char>, T::Position: Copy + Debug]
    {
        expression_0()
    }
}

pub fn ksx_term<T>() -> impl Parser<T, Output = ExpressionRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
{
    unary_operation(|| function_call(expression_8(), expression()))
}

#[cfg(test)]
mod tests {
    use crate::{
        expression::{
            expression, primitive::Primitive, BinaryOperator, Expression, ExpressionRaw,
            UnaryOperator,
        },
        range::Range,
        statement::Statement,
    };
    use combine::Parser;

    #[test]
    fn primitive() {
        let parse = |s| expression().parse(s);

        assert_eq!(
            parse("nil").unwrap().0,
            ExpressionRaw(Expression::Primitive(Primitive::Nil), Range::str(1, 1))
        );
        assert_eq!(
            parse("true").unwrap().0,
            ExpressionRaw(
                Expression::Primitive(Primitive::Boolean(true)),
                Range::str(1, 1)
            )
        );
        assert_eq!(
            parse("false").unwrap().0,
            ExpressionRaw(
                Expression::Primitive(Primitive::Boolean(false)),
                Range::str(1, 1)
            )
        );
        assert_eq!(
            parse("123").unwrap().0,
            ExpressionRaw(
                Expression::Primitive(Primitive::Integer(123)),
                Range::str(1, 1)
            )
        );
        assert_eq!(
            parse("123.456").unwrap().0,
            ExpressionRaw(
                Expression::Primitive(Primitive::Float(123.456, 6)),
                Range::str(1, 1)
            )
        );
        assert_eq!(
            parse("\"foo\"").unwrap().0,
            ExpressionRaw(
                Expression::Primitive(Primitive::String(String::from("foo"))),
                Range::str(1, 1)
            )
        );
    }

    #[test]
    fn identifier() {
        let parse = |s| expression().parse(s);

        assert_eq!(
            parse("foo").unwrap().0,
            ExpressionRaw(
                Expression::Identifier(String::from("foo")),
                Range::str(1, 1)
            )
        );
    }

    #[test]
    fn group() {
        let parse = |s| expression().parse(s);

        assert_eq!(
            parse("(nil)").unwrap().0,
            ExpressionRaw(
                Expression::Group(Box::new(ExpressionRaw(
                    Expression::Primitive(Primitive::Nil),
                    Range::str(1, 1)
                ))),
                Range::str(1, 1)
            )
        );
    }

    #[test]
    fn closure() {
        let parse = |s| expression().parse(s);

        assert_eq!(
            parse("{ nil; nil }").unwrap().0,
            ExpressionRaw(
                Expression::Closure(vec![
                    Statement::Effect(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                    Statement::Effect(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    ))
                ]),
                Range::str(1, 1)
            )
        );
        assert_eq!(
            parse("{}").unwrap().0,
            ExpressionRaw(Expression::Closure(vec![]), Range::str(1, 1))
        );
    }

    #[test]
    fn unary_operation() {
        let parse = |s| expression().parse(s);

        assert_eq!(
            parse("!nil").unwrap().0,
            ExpressionRaw(
                Expression::UnaryOperation(
                    UnaryOperator::Not,
                    Box::new(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                ),
                Range::str(1, 1)
            )
        );
        assert_eq!(
            parse("+nil").unwrap().0,
            ExpressionRaw(
                Expression::UnaryOperation(
                    UnaryOperator::Absolute,
                    Box::new(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                ),
                Range::str(1, 1)
            )
        );
        assert_eq!(
            parse("-nil").unwrap().0,
            ExpressionRaw(
                Expression::UnaryOperation(
                    UnaryOperator::Negate,
                    Box::new(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                ),
                Range::str(1, 1)
            )
        );
    }

    #[test]
    fn binary_operation() {
        let parse = |s| expression().parse(s);

        assert_eq!(
            parse("nil + nil").unwrap().0,
            ExpressionRaw(
                Expression::BinaryOperation(
                    BinaryOperator::Add,
                    Box::new(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                    Box::new(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                ),
                Range::str(1, 1)
            )
        );
        assert_eq!(
            parse("nil - nil").unwrap().0,
            ExpressionRaw(
                Expression::BinaryOperation(
                    BinaryOperator::Subtract,
                    Box::new(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                    Box::new(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                ),
                Range::str(1, 1)
            )
        );
        assert_eq!(
            parse("nil * nil").unwrap().0,
            ExpressionRaw(
                Expression::BinaryOperation(
                    BinaryOperator::Multiply,
                    Box::new(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                    Box::new(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                ),
                Range::str(1, 1)
            )
        );
        assert_eq!(
            parse("nil / nil").unwrap().0,
            ExpressionRaw(
                Expression::BinaryOperation(
                    BinaryOperator::Divide,
                    Box::new(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                    Box::new(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                ),
                Range::str(1, 1)
            )
        );
        assert_eq!(
            parse("nil ^ nil").unwrap().0,
            ExpressionRaw(
                Expression::BinaryOperation(
                    BinaryOperator::Exponent,
                    Box::new(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                    Box::new(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                ),
                Range::str(1, 1)
            )
        );
        assert_eq!(
            parse("nil < nil").unwrap().0,
            ExpressionRaw(
                Expression::BinaryOperation(
                    BinaryOperator::LessThan,
                    Box::new(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                    Box::new(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                ),
                Range::str(1, 1)
            )
        );
        assert_eq!(
            parse("nil <= nil").unwrap().0,
            ExpressionRaw(
                Expression::BinaryOperation(
                    BinaryOperator::LessThanOrEqual,
                    Box::new(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                    Box::new(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                ),
                Range::str(1, 1)
            )
        );
        assert_eq!(
            parse("nil > nil").unwrap().0,
            ExpressionRaw(
                Expression::BinaryOperation(
                    BinaryOperator::GreaterThan,
                    Box::new(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                    Box::new(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                ),
                Range::str(1, 1)
            )
        );
        assert_eq!(
            parse("nil >= nil").unwrap().0,
            ExpressionRaw(
                Expression::BinaryOperation(
                    BinaryOperator::GreaterThanOrEqual,
                    Box::new(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                    Box::new(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                ),
                Range::str(1, 1)
            )
        );
        assert_eq!(
            parse("nil == nil").unwrap().0,
            ExpressionRaw(
                Expression::BinaryOperation(
                    BinaryOperator::Equal,
                    Box::new(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                    Box::new(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                ),
                Range::str(1, 1)
            )
        );
        assert_eq!(
            parse("nil != nil").unwrap().0,
            ExpressionRaw(
                Expression::BinaryOperation(
                    BinaryOperator::NotEqual,
                    Box::new(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                    Box::new(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                ),
                Range::str(1, 1)
            )
        );
        assert_eq!(
            parse("nil && nil").unwrap().0,
            ExpressionRaw(
                Expression::BinaryOperation(
                    BinaryOperator::And,
                    Box::new(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                    Box::new(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                ),
                Range::str(1, 1)
            )
        );
        assert_eq!(
            parse("nil || nil").unwrap().0,
            ExpressionRaw(
                Expression::BinaryOperation(
                    BinaryOperator::Or,
                    Box::new(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                    Box::new(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                ),
                Range::str(1, 1)
            )
        );
    }

    #[test]
    fn dot_access() {
        let parse = |s| expression().parse(s);

        assert_eq!(
            parse("nil.foo").unwrap().0,
            ExpressionRaw(
                Expression::DotAccess(
                    Box::new(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                    String::from("foo")
                ),
                Range::str(1, 1)
            )
        );
    }

    #[test]
    fn function_call() {
        let parse = |s| expression().parse(s);

        assert_eq!(
            parse("nil(nil, nil)").unwrap().0,
            ExpressionRaw(
                Expression::FunctionCall(
                    Box::new(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                    vec![
                        ExpressionRaw(Expression::Primitive(Primitive::Nil), Range::str(1, 1)),
                        ExpressionRaw(Expression::Primitive(Primitive::Nil), Range::str(1, 1))
                    ]
                ),
                Range::str(1, 1)
            )
        );
        assert_eq!(
            parse("nil()").unwrap().0,
            ExpressionRaw(
                Expression::FunctionCall(
                    Box::new(ExpressionRaw(
                        Expression::Primitive(Primitive::Nil),
                        Range::str(1, 1)
                    )),
                    vec![]
                ),
                Range::str(1, 1)
            )
        );
    }

    #[test]
    fn style() {
        let parse = |s| expression().parse(s);

        assert_eq!(
            parse("style { foo: nil }").unwrap().0,
            ExpressionRaw(
                Expression::Style(vec![(
                    String::from("foo"),
                    ExpressionRaw(Expression::Primitive(Primitive::Nil), Range::str(1, 1))
                )]),
                Range::str(1, 1)
            )
        );
    }
}
