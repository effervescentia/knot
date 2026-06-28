pub mod binary_operation;
pub mod primitive;
pub mod style;

use crate::{
    component,
    matcher::{self as m, Position},
    statement,
};
use combine::{choice, many, parser, position, Parser, Stream};
use lang::ast;

fn primitive<T>() -> impl Parser<T, Output = ast::raw::Expression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    m::lexeme(primitive::primitive())
        .map(|(x, range)| ast::raw::Expression::raw(ast::Expression::Primitive(x), range))
}

fn group<T, P>(parser: P) -> impl Parser<T, Output = ast::raw::Expression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
    P: Parser<T, Output = ast::raw::Expression>,
{
    m::group(parser)
        .map(|(x, range)| ast::raw::Expression::raw(ast::Expression::Group(Box::new(x)), range))
}

fn identifier<T>() -> impl Parser<T, Output = ast::raw::Expression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    m::standard_identifier()
        .map(|(x, range)| ast::raw::Expression::raw(ast::Expression::Identifier(x), range))
}

fn closure<T, P>(parser: impl Fn() -> P) -> impl Parser<T, Output = ast::raw::Expression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
    P: Parser<T, Output = ast::raw::Expression>,
{
    m::closure(many::<Vec<_>, _, _>(statement::statement(parser)))
        .map(|(xs, range)| ast::raw::Expression::raw(ast::Expression::Closure(xs), range))
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

            ast::raw::Expression::raw(ast::Expression::UnaryOperation(op, Box::new(x)), range)
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

            ast::raw::Expression::raw(ast::Expression::PropertyAccess(Box::new(lhs), rhs), range)
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
    m::folding(lhs, m::tuple(rhs), |acc, (args, end)| {
        let range = acc.0.range() + &end;

        ast::raw::Expression::raw(ast::Expression::FunctionCall(Box::new(acc), args), range)
    })
}

fn component<T>() -> impl Parser<T, Output = ast::raw::Expression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    component::component().map(|x| {
        let range = *x.0.range();

        ast::raw::Expression::raw(ast::Expression::Component(Box::new(x)), range)
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
    use combine::{eof, stream::position::Stream, EasyParser, Parser};

    fn parse(s: &str) -> crate::Result<lang::ast::raw::Expression> {
        super::expression().skip(eof()).easy_parse(Stream::new(s))
    }

    #[test]
    fn primitive_nil() {
        let ast = parse("nil").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn primitive_true() {
        let ast = parse("true").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn primitive_false() {
        let ast = parse("false").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn primitive_integer() {
        let ast = parse("123").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn primitive_float() {
        let ast = parse("123.456").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn primitive_string() {
        let ast = parse("\"foo\"").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn identifier() {
        let ast = parse("foo").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn group() {
        let ast = parse("(nil)").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn closure_empty() {
        let ast = parse("{}").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn closure_with_statements() {
        let ast = parse("{ nil; nil }").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn unary_not_operation() {
        let ast = parse("!nil").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn unary_absolute_operation() {
        let ast = parse("+nil").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn unary_negative_operation() {
        let ast = parse("-nil").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn binary_add_operation() {
        let ast = parse("nil + nil").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn binary_subtract_operation() {
        let ast = parse("nil - nil").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn binary_multiply_operation() {
        let ast = parse("nil * nil").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn binary_divide_operation() {
        let ast = parse("nil / nil").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn binary_exponent_operation() {
        let ast = parse("nil ^ nil").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn binary_less_than_operation() {
        let ast = parse("nil < nil").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn binary_less_than_or_equal_operation() {
        let ast = parse("nil <= nil").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn binary_greater_than_operation() {
        let ast = parse("nil > nil").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn binary_greater_than_or_equal_operation() {
        let ast = parse("nil >= nil").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn binary_equal_operation() {
        let ast = parse("nil == nil").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn binary_unequal_operation() {
        let ast = parse("nil != nil").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn binary_and_operation() {
        let ast = parse("nil && nil").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn binary_or_operation() {
        let ast = parse("nil || nil").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn property_access() {
        let ast = parse("nil.foo").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn function_call_empty() {
        let ast = parse("nil()").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn function_call() {
        let ast = parse("nil(nil, nil)").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn style_empty() {
        let ast = parse("style {}").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn style() {
        let ast = parse("style { foo: nil }").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }

    #[test]
    fn element() {
        let ast = parse("<foo />").unwrap().0;

        insta::assert_debug_snapshot!(ast);
    }
}
