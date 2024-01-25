use crate::{ast, matcher as m};
use combine::{chainl1, chainr1, choice, Parser, Stream};

fn binary_operation<U>(
    o: ast::BinaryOperator,
) -> impl FnMut(U) -> Box<dyn Fn(ast::raw::Expression, ast::raw::Expression) -> ast::raw::Expression>
{
    move |_| {
        Box::new(move |lhs, rhs| {
            let range = lhs.0.range() + rhs.0.range();

            ast::raw::Expression::new(
                ast::Expression::BinaryOperation(o, Box::new(lhs), Box::new(rhs)),
                range,
            )
        })
    }
}

pub fn logical<T, P>(parser: P) -> impl Parser<T, Output = ast::raw::Expression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
    P: Parser<T, Output = ast::raw::Expression>,
{
    let and = || {
        chainl1(
            parser,
            m::glyph("&&").map(binary_operation(ast::BinaryOperator::And)),
        )
    };
    let or = || {
        chainl1(
            and(),
            m::glyph("||").map(binary_operation(ast::BinaryOperator::Or)),
        )
    };

    or()
}

pub fn comparative<T, P>(parser: P) -> impl Parser<T, Output = ast::raw::Expression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
    P: Parser<T, Output = ast::raw::Expression>,
{
    chainl1(
        parser,
        choice((
            m::glyph("==").map(binary_operation(ast::BinaryOperator::Equal)),
            m::glyph("!=").map(binary_operation(ast::BinaryOperator::NotEqual)),
        )),
    )
}

pub fn relational<T, P>(parser: P) -> impl Parser<T, Output = ast::raw::Expression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
    P: Parser<T, Output = ast::raw::Expression>,
{
    chainl1(
        parser,
        choice((
            m::glyph("<=").map(binary_operation(ast::BinaryOperator::LessThanOrEqual)),
            m::symbol('<').map(binary_operation(ast::BinaryOperator::LessThan)),
            m::glyph(">=").map(binary_operation(ast::BinaryOperator::GreaterThanOrEqual)),
            m::symbol('>').map(binary_operation(ast::BinaryOperator::GreaterThan)),
        )),
    )
}

pub fn arithmetic<T, P>(parser: P) -> impl Parser<T, Output = ast::raw::Expression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
    P: Parser<T, Output = ast::raw::Expression>,
{
    let exponent = || {
        chainr1(
            parser,
            m::symbol('^').map(binary_operation(ast::BinaryOperator::Exponent)),
        )
    };
    let multiply_or_divide = || {
        chainl1(
            exponent(),
            choice((
                m::symbol('*').map(binary_operation(ast::BinaryOperator::Multiply)),
                m::symbol('/').map(binary_operation(ast::BinaryOperator::Divide)),
            )),
        )
    };
    let add_or_subtract = || {
        chainl1(
            multiply_or_divide(),
            choice((
                m::symbol('+').map(binary_operation(ast::BinaryOperator::Add)),
                m::symbol('-').map(binary_operation(ast::BinaryOperator::Subtract)),
            )),
        )
    };

    add_or_subtract()
}
