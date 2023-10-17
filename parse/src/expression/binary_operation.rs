use crate::matcher as m;
use combine::{chainl1, chainr1, choice, Parser, Stream};
use knot_language::{
    ast::{BinaryOperator, Expression, ExpressionNode},
    Position,
};

fn binary_operation<T, U>(
    o: BinaryOperator,
) -> impl FnMut(U) -> Box<dyn Fn(ExpressionNode<()>, ExpressionNode<()>) -> ExpressionNode<()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
{
    move |_| {
        Box::new(move |lhs, rhs| {
            let range = lhs.node().range() + rhs.node().range();

            ExpressionNode::raw(
                Expression::BinaryOperation(o, Box::new(lhs), Box::new(rhs)),
                range,
            )
        })
    }
}

pub fn logical<T, P>(parser: P) -> impl Parser<T, Output = ExpressionNode<()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
    P: Parser<T, Output = ExpressionNode<()>>,
{
    let and = || {
        chainl1(
            parser,
            m::glyph("&&").map(binary_operation::<T, _>(BinaryOperator::And)),
        )
    };
    let or = || {
        chainl1(
            and(),
            m::glyph("||").map(binary_operation::<T, _>(BinaryOperator::Or)),
        )
    };

    or()
}

pub fn comparative<T, P>(parser: P) -> impl Parser<T, Output = ExpressionNode<()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
    P: Parser<T, Output = ExpressionNode<()>>,
{
    chainl1(
        parser,
        choice((
            m::glyph("==").map(binary_operation::<T, _>(BinaryOperator::Equal)),
            m::glyph("!=").map(binary_operation::<T, _>(BinaryOperator::NotEqual)),
        )),
    )
}

pub fn relational<T, P>(parser: P) -> impl Parser<T, Output = ExpressionNode<()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
    P: Parser<T, Output = ExpressionNode<()>>,
{
    chainl1(
        parser,
        choice((
            m::glyph("<=").map(binary_operation::<T, _>(BinaryOperator::LessThanOrEqual)),
            m::symbol('<').map(binary_operation::<T, _>(BinaryOperator::LessThan)),
            m::glyph(">=").map(binary_operation::<T, _>(BinaryOperator::GreaterThanOrEqual)),
            m::symbol('>').map(binary_operation::<T, _>(BinaryOperator::GreaterThan)),
        )),
    )
}

pub fn arithmetic<T, P>(parser: P) -> impl Parser<T, Output = ExpressionNode<()>>
where
    T: Stream<Token = char>,
    T::Position: Position,
    P: Parser<T, Output = ExpressionNode<()>>,
{
    let exponent = || {
        chainr1(
            parser,
            m::symbol('^').map(binary_operation::<T, _>(BinaryOperator::Exponent)),
        )
    };
    let multiply_or_divide = || {
        chainl1(
            exponent(),
            choice((
                m::symbol('*').map(binary_operation::<T, _>(BinaryOperator::Multiply)),
                m::symbol('/').map(binary_operation::<T, _>(BinaryOperator::Divide)),
            )),
        )
    };
    let add_or_subtract = || {
        chainl1(
            multiply_or_divide(),
            choice((
                m::symbol('+').map(binary_operation::<T, _>(BinaryOperator::Add)),
                m::symbol('-').map(binary_operation::<T, _>(BinaryOperator::Subtract)),
            )),
        )
    };

    add_or_subtract()
}
