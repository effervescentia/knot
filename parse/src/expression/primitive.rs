use combine::{
    attempt, between, choice, many, many1, none_of, parser::char as p, token, value, Parser, Stream,
};
use knot_language::ast::Primitive;

fn nil<T>() -> impl Parser<T, Output = Primitive>
where
    T: Stream<Token = char>,
{
    p::string("nil").with(value(Primitive::Nil))
}

fn boolean<T>() -> impl Parser<T, Output = Primitive>
where
    T: Stream<Token = char>,
{
    p::string("true")
        .with(value(true))
        .or(p::string("false").with(value(false)))
        .map(Primitive::Boolean)
}

fn integer<T>() -> impl Parser<T, Output = Primitive>
where
    T: Stream<Token = char>,
{
    many1::<String, _, _>(p::digit())
        .map(|chars| chars.parse::<i64>().unwrap())
        .map(Primitive::Integer)
}

fn float<T>() -> impl Parser<T, Output = Primitive>
where
    T: Stream<Token = char>,
{
    let integer = many1::<String, _, _>(p::digit());
    let fraction = many1::<String, _, _>(p::digit());

    attempt((integer, token('.'), fraction)).map(|(integer, _, fraction)| {
        if fraction.is_empty() {
            Primitive::Float(integer.parse::<f64>().unwrap(), integer.len() as i32)
        } else {
            let float = format!("{}.{}", integer, fraction);

            Primitive::Float(float.parse::<f64>().unwrap(), fraction.len() as i32)
        }
    })
}

fn string<T>() -> impl Parser<T, Output = Primitive>
where
    T: Stream<Token = char>,
{
    between(token('"'), token('"'), many(none_of(vec!['"', '\\']))).map(Primitive::String)
}

pub fn primitive<T>() -> impl Parser<T, Output = Primitive>
where
    T: Stream<Token = char>,
{
    choice((nil(), boolean(), float(), integer(), string()))
}

#[cfg(test)]
mod tests {
    use super::{primitive, Primitive};
    use combine::{eof, error::StringStreamError, Parser};

    fn parse(s: &str) -> Result<(Primitive, &str), StringStreamError> {
        primitive().skip(eof()).parse(s)
    }

    #[test]
    fn nil() {
        assert_eq!(parse("nil").unwrap().0, Primitive::Nil);
    }

    #[test]
    fn boolean_true() {
        assert_eq!(parse("true").unwrap().0, Primitive::Boolean(true));
    }

    #[test]
    fn boolean_false() {
        assert_eq!(parse("false").unwrap().0, Primitive::Boolean(false));
    }

    #[test]
    fn integer() {
        assert_eq!(parse("0").unwrap().0, Primitive::Integer(0));
        assert_eq!(parse("123").unwrap().0, Primitive::Integer(123));
    }

    #[test]
    fn float() {
        assert_eq!(parse("123.456").unwrap().0, Primitive::Float(123.456, 3));
    }

    #[test]
    fn string() {
        assert_eq!(
            parse("\"foo\"").unwrap().0,
            Primitive::String(String::from("foo"))
        );
    }
}
