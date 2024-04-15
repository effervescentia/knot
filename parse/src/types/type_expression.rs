use crate::matcher as m;
use combine::{attempt, choice, parser, sep_end_by, Parser, Stream};
use lang::ast;

fn primitive<T>() -> impl Parser<T, Output = ast::raw::TypeExpression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    fn bind<U>(
        s: &'static str,
        f: impl Fn() -> ast::TypeExpression<ast::raw::Binding, ast::raw::TypeExpression>,
    ) -> impl Parser<U, Output = ast::raw::TypeExpression>
    where
        U: Stream<Token = char>,
        U::Position: m::Position,
    {
        m::keyword(s).map(move |(_, range)| ast::raw::TypeExpression::raw(f(), range))
    }

    choice((
        bind("nil", || {
            ast::TypeExpression::Primitive(ast::TypePrimitive::Nil)
        }),
        bind("boolean", || {
            ast::TypeExpression::Primitive(ast::TypePrimitive::Boolean)
        }),
        bind("integer", || {
            ast::TypeExpression::Primitive(ast::TypePrimitive::Integer)
        }),
        bind("float", || {
            ast::TypeExpression::Primitive(ast::TypePrimitive::Float)
        }),
        bind("string", || {
            ast::TypeExpression::Primitive(ast::TypePrimitive::String)
        }),
        bind("style", || {
            ast::TypeExpression::Primitive(ast::TypePrimitive::Style)
        }),
        bind("element", || {
            ast::TypeExpression::Primitive(ast::TypePrimitive::Element)
        }),
    ))
}

fn identifier<T>() -> impl Parser<T, Output = ast::raw::TypeExpression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    m::standard_identifier()
        .map(|(x, range)| ast::raw::TypeExpression::raw(ast::TypeExpression::Identifier(x), range))
}

fn group<T, P>(parser: P) -> impl Parser<T, Output = ast::raw::TypeExpression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
    P: Parser<T, Output = ast::raw::TypeExpression>,
{
    m::between(m::symbol('('), m::symbol(')'), parser).map(|(inner, range)| {
        ast::raw::TypeExpression::raw(ast::TypeExpression::Group(Box::new(inner)), range)
    })
}

fn dot_access<T, P>(parser: P) -> impl Parser<T, Output = ast::raw::TypeExpression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
    P: Parser<T, Output = ast::raw::TypeExpression>,
{
    m::folding(
        parser,
        m::symbol('.').with(m::standard_identifier()),
        |lhs, (rhs, end)| {
            let range = lhs.0.range() + &end;
            ast::raw::TypeExpression::raw(
                ast::TypeExpression::PropertyAccess(Box::new(lhs), rhs),
                range,
            )
        },
    )
}

fn object<T, P>(parser: impl Fn() -> P) -> impl Parser<T, Output = ast::raw::TypeExpression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
    P: Parser<T, Output = ast::raw::TypeExpression>,
{
    let required_entry = (m::binding(), m::symbol(':'), parser())
        .map(|(binding, _, x)| ast::ObjectTypeExpressionEntry::Required(binding, x));

    let optional_entry = (m::binding(), m::glyph("?:"), parser())
        .map(|(binding, _, x)| ast::ObjectTypeExpressionEntry::Optional(binding, x));

    let spread_entry =
        (m::glyph("..."), parser()).map(|(_, x)| ast::ObjectTypeExpressionEntry::Spread(x));

    m::between(
        m::symbol('{'),
        m::symbol('}'),
        sep_end_by(
            choice((
                attempt(required_entry),
                attempt(optional_entry),
                spread_entry,
            )),
            m::symbol(','),
        ),
    )
    .map(|(entries, range)| {
        ast::raw::TypeExpression::raw(ast::TypeExpression::Object(entries), range)
    })
}

fn function<T, P>(parser: impl Fn() -> P) -> impl Parser<T, Output = ast::raw::TypeExpression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
    P: Parser<T, Output = ast::raw::TypeExpression>,
{
    (
        attempt(
            m::between(
                m::symbol('('),
                m::symbol(')'),
                sep_end_by(parser(), m::symbol(',')),
            )
            .skip(m::glyph("->")),
        ),
        parser(),
    )
        .map(|((parameters, start), result)| {
            let range = &start + result.0.range();
            ast::raw::TypeExpression::raw(
                ast::TypeExpression::Function(parameters, Box::new(result)),
                range,
            )
        })
}

fn type_expression_2<T>() -> impl Parser<T, Output = ast::raw::TypeExpression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    choice((
        function(type_expression),
        group(type_expression()),
        object(type_expression),
        primitive(),
        identifier(),
    ))
}

fn type_expression_1<T>() -> impl Parser<T, Output = ast::raw::TypeExpression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    dot_access(type_expression_2())
}

// TODO: use this for lists ([], [][][])
fn type_expression_0<T>() -> impl Parser<T, Output = ast::raw::TypeExpression>
where
    T: Stream<Token = char>,
    T::Position: m::Position,
{
    type_expression_1()
}

parser! {
    pub fn type_expression[T]()(T) -> ast::raw::TypeExpression
    where
        [T: Stream<Token = char>, T::Position: m::Position]
    {
        type_expression_0()
    }
}

#[cfg(test)]
mod tests {
    use combine::{stream::position::Stream, EasyParser};
    use kore::{assert_eq, str};
    use lang::{ast, Range};

    fn parse(s: &str) -> crate::Result<ast::raw::TypeExpression> {
        super::type_expression().easy_parse(Stream::new(s))
    }

    #[test]
    fn nil() {
        assert_eq!(
            parse("nil").unwrap().0,
            ast::raw::TypeExpression::raw(
                ast::TypeExpression::Primitive(ast::TypePrimitive::Nil),
                Range::new((1, 1), (1, 3))
            )
        );
    }

    #[test]
    fn boolean() {
        assert_eq!(
            parse("boolean").unwrap().0,
            ast::raw::TypeExpression::raw(
                ast::TypeExpression::Primitive(ast::TypePrimitive::Boolean),
                Range::new((1, 1), (1, 7))
            )
        );
    }

    #[test]
    fn integer() {
        assert_eq!(
            parse("integer").unwrap().0,
            ast::raw::TypeExpression::raw(
                ast::TypeExpression::Primitive(ast::TypePrimitive::Integer),
                Range::new((1, 1), (1, 7))
            )
        );
    }

    #[test]
    fn float() {
        assert_eq!(
            parse("float").unwrap().0,
            ast::raw::TypeExpression::raw(
                ast::TypeExpression::Primitive(ast::TypePrimitive::Float),
                Range::new((1, 1), (1, 5))
            )
        );
    }

    #[test]
    fn string() {
        assert_eq!(
            parse("string").unwrap().0,
            ast::raw::TypeExpression::raw(
                ast::TypeExpression::Primitive(ast::TypePrimitive::String),
                Range::new((1, 1), (1, 6))
            )
        );
    }

    #[test]
    fn style() {
        assert_eq!(
            parse("style").unwrap().0,
            ast::raw::TypeExpression::raw(
                ast::TypeExpression::Primitive(ast::TypePrimitive::Style),
                Range::new((1, 1), (1, 5))
            )
        );
    }

    #[test]
    fn element() {
        assert_eq!(
            parse("element").unwrap().0,
            ast::raw::TypeExpression::raw(
                ast::TypeExpression::Primitive(ast::TypePrimitive::Element),
                Range::new((1, 1), (1, 7))
            )
        );
    }

    #[test]
    fn identifier() {
        assert_eq!(
            parse("foo").unwrap().0,
            ast::raw::TypeExpression::raw(
                ast::TypeExpression::Identifier(str!("foo")),
                Range::new((1, 1), (1, 3))
            )
        );
    }

    #[test]
    fn group() {
        assert_eq!(
            parse("(nil)").unwrap().0,
            ast::raw::TypeExpression::raw(
                ast::TypeExpression::Group(Box::new(ast::raw::TypeExpression::raw(
                    ast::TypeExpression::Primitive(ast::TypePrimitive::Nil),
                    Range::new((1, 2), (1, 4))
                ))),
                Range::new((1, 1), (1, 5))
            )
        );
    }

    #[test]
    fn function() {
        assert_eq!(
            parse("(nil, boolean) -> nil").unwrap().0,
            ast::raw::TypeExpression::raw(
                ast::TypeExpression::Function(
                    vec![
                        ast::raw::TypeExpression::raw(
                            ast::TypeExpression::Primitive(ast::TypePrimitive::Nil),
                            Range::new((1, 2), (1, 4))
                        ),
                        ast::raw::TypeExpression::raw(
                            ast::TypeExpression::Primitive(ast::TypePrimitive::Boolean),
                            Range::new((1, 7), (1, 13))
                        )
                    ],
                    Box::new(ast::raw::TypeExpression::raw(
                        ast::TypeExpression::Primitive(ast::TypePrimitive::Nil),
                        Range::new((1, 19), (1, 21))
                    ))
                ),
                Range::new((1, 1), (1, 21))
            )
        );
    }

    #[test]
    fn function_empty_parameters() {
        assert_eq!(
            parse("() -> nil").unwrap().0,
            ast::raw::TypeExpression::raw(
                ast::TypeExpression::Function(
                    vec![],
                    Box::new(ast::raw::TypeExpression::raw(
                        ast::TypeExpression::Primitive(ast::TypePrimitive::Nil),
                        Range::new((1, 7), (1, 9))
                    ))
                ),
                Range::new((1, 1), (1, 9))
            )
        );
    }

    #[test]
    fn dot_access() {
        assert_eq!(
            parse("nil.foo").unwrap().0,
            ast::raw::TypeExpression::raw(
                ast::TypeExpression::PropertyAccess(
                    Box::new(ast::raw::TypeExpression::raw(
                        ast::TypeExpression::Primitive(ast::TypePrimitive::Nil),
                        Range::new((1, 1), (1, 3))
                    )),
                    str!("foo")
                ),
                Range::new((1, 1), (1, 7))
            )
        );
    }

    #[test]
    fn empty_object() {
        assert_eq!(
            parse("{}").unwrap().0,
            ast::raw::TypeExpression::raw(
                ast::TypeExpression::Object(vec![]),
                Range::new((1, 1), (1, 2))
            )
        );
    }

    #[test]
    fn object() {
        assert_eq!(
            parse(
                "{
  foo: integer,
  bar?: boolean,
  ...fizz
}"
            )
            .unwrap()
            .0,
            ast::raw::TypeExpression::raw(
                ast::TypeExpression::Object(vec![
                    ast::ObjectTypeExpressionEntry::Required(
                        ast::raw::Binding::new(
                            ast::Binding(str!("foo")),
                            Range::new((2, 3), (2, 5))
                        ),
                        ast::raw::TypeExpression::raw(
                            ast::TypeExpression::Primitive(ast::TypePrimitive::Integer),
                            Range::new((2, 8), (2, 14))
                        )
                    ),
                    ast::ObjectTypeExpressionEntry::Optional(
                        ast::raw::Binding::new(
                            ast::Binding(str!("bar")),
                            Range::new((3, 3), (3, 5))
                        ),
                        ast::raw::TypeExpression::raw(
                            ast::TypeExpression::Primitive(ast::TypePrimitive::Boolean),
                            Range::new((3, 9), (3, 15))
                        )
                    ),
                    ast::ObjectTypeExpressionEntry::Spread(ast::raw::TypeExpression::raw(
                        ast::TypeExpression::Identifier(str!("fizz")),
                        Range::new((4, 6), (4, 9))
                    ))
                ]),
                Range::new((1, 1), (5, 1))
            )
        );
    }
}
