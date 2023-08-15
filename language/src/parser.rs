extern crate combine;

#[derive(Debug, PartialEq)]
enum Primitive {
    Nil,
    Boolean(bool),
    Integer(i64),
    Float(f64, i32),
    String(String),
}

mod primitive {
    use combine::parser::char::{digit, letter, spaces};
    use combine::{many1, ParseError, Parser, Stream};

    use super::Primitive;

    // pub fn parse<I>() -> impl Parser<I, Output = Primitive>
    // where
    //     I: Stream<Token = char>,
    // {
    //     let integer = many1(letter());

    //     println!("primitive::parse()");

    //     integer
    // }
}
