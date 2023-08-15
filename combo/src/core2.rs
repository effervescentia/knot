use std::{
    cell::{Cell, RefCell},
    rc::Rc,
};

use crate::{Cursor, Stream};

// pub struct Parser<'a, T: 'a> {
//     parse: Box<dyn Fn(&'a mut Stream<char>, Rc<Cursor>) -> Option<(T, Rc<Cursor>)> + 'a>,
// }

// impl<'a, T> Parser<'a, T> {
//     fn new(
//         parse: impl Fn(&'a mut Stream<char>, Rc<Cursor>) -> Option<(T, Rc<Cursor>)> + 'a,
//     ) -> Self {
//         Self {
//             parse: Box::new(parse),
//         }
//     }

//     pub fn parse(
//         &self,
//         input: &'a mut Stream<char>,
//         cursor: Rc<Cursor>,
//     ) -> Option<(T, Rc<Cursor>)> {
//         // let input = input.get_mut();
//         let cursor = cursor.duplicate(input);

//         (self.parse)(input, Rc::new(cursor))
//     }

//     pub fn then<R>(
//         self,
//         then: impl Fn(Option<(T, Rc<Cursor>)>) -> Option<(R, Rc<Cursor>)> + 'a,
//     ) -> Parser<'a, R> {
//         Parser::new(move |input, cursor| then(self.parse(input, cursor)))
//     }

//     pub fn bind<R>(
//         self,
//         bind: impl Fn((T, Rc<Cursor>)) -> Option<(R, Rc<Cursor>)> + 'a,
//     ) -> Parser<'a, R> {
//         self.then(move |result| result.and_then(|value| bind(value)))
//     }

//     pub fn map<R>(self, map: impl Fn(T) -> R + 'a) -> Parser<'a, R> {
//         self.bind(move |(result, cursor)| Some((map(result), cursor)))
//     }

//     // pub fn many(self) -> Parser<'a, Vec<T>> {
//     //     self.bind(|(x, _)| Some(self.many().bind(|(xs, _)| vec![x])))
//     // }

//     pub fn or_else(self, else_parser: Parser<'a, T>) -> Parser<'a, T> {
//         // self.then(move |result| result.or_else(|| bind(value)))
//         Parser::new(
//             move |input, cursor| match self.parse(input, cursor.clone()) {
//                 None => else_parser.parse(input, cursor),
//                 result => result,
//             },
//         )
//         // self.then(move |result| result.or_else(else_parser.parse()))
//         // self.then(move |result| Some(result.unwrap_or(vec![])))
//     }
// }

pub struct Parser<'a, T: 'a> {
    parse: Box<dyn Fn(&RefCell<Stream<char>>, Rc<Cursor>) -> Option<(T, Rc<Cursor>)> + 'a>,
}

impl<'a, T> Parser<'a, T> {
    fn new(
        parse: impl Fn(&RefCell<Stream<char>>, Rc<Cursor>) -> Option<(T, Rc<Cursor>)> + 'a,
    ) -> Self {
        Self {
            parse: Box::new(parse),
        }
    }

    pub fn parse(
        &self,
        input: &RefCell<Stream<char>>,
        cursor: Rc<Cursor>,
    ) -> Option<(T, Rc<Cursor>)> {
        // let input = input.get_mut();
        // let cursor = cursor.duplicate(input);

        (self.parse)(input, cursor)
    }

    pub fn then<R>(
        self,
        then: impl Fn(Option<(T, Rc<Cursor>)>) -> Option<(R, Rc<Cursor>)> + 'a,
    ) -> Parser<'a, R> {
        Parser::new(move |input, cursor| then(self.parse(input, cursor)))
    }

    pub fn bind<R>(
        self,
        bind: impl Fn((T, Rc<Cursor>)) -> Option<(R, Rc<Cursor>)> + 'a,
    ) -> Parser<'a, R> {
        self.then(move |result| result.and_then(|value| bind(value)))
    }

    pub fn map<R>(self, map: impl Fn(T) -> R + 'a) -> Parser<'a, R> {
        self.bind(move |(result, cursor)| Some((map(result), cursor)))
    }

    // pub fn many(self) -> Parser<'a, Vec<T>> {
    //     self.bind(|(x, _)| Some(self.many().bind(|(xs, _)| vec![x])))
    // }

    pub fn or_else(self, else_parser: Parser<'a, T>) -> Parser<'a, T> {
        // self.then(move |result| result.or_else(|| bind(value)))
        Parser::new(
            move |input, cursor| match self.parse(input, cursor.clone()) {
                None => else_parser.parse(input, cursor),
                result => result,
            },
        )
        // self.then(move |result| result.or_else(else_parser.parse()))
        // self.then(move |result| Some(result.unwrap_or(vec![])))
    }
}

pub mod matcher {
    use super::*;

    // pub fn any<'a>() -> Parser<'a, &'a char> {
    //     Parser::new(
    //         |input, cursor| match input.borrow_mut().next(cursor.as_ref()) {
    //             Some(result) => Some((result, cursor)),
    //             None => None,
    //         },
    //     )
    // }

    // pub fn eof<'a, T>(value: &'a T) -> Parser<'a, &'a T> {
    //     Parser::new(
    //         move |input, cursor| match input.borrow_mut().next(cursor.as_ref()) {
    //             None => Some((value, cursor)),
    //             Some(_) => None,
    //         },
    //     )
    // }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::fmt::{Debug, Display};

    fn parse<'a, T>(parser: Parser<'a, T>, input: Stream<char>) -> Option<(T, Rc<Cursor>)> {
        let cursor = Rc::new(Cursor::new(&input));
        let cell = RefCell::new(input);
        parser.parse(&cell, cursor)
    }

    fn assert_result<T: Debug + Display + PartialEq>(
        option: &Option<(T, Rc<Cursor>)>,
        expected: T,
    ) {
        match option {
            Some((result, _)) => assert_eq!(result, &expected),
            None => panic!("expected result {}", expected),
        }
    }

    fn assert_position<T: Debug + Display + PartialEq>(
        option: &Option<(T, Rc<Cursor>)>,
        expected: usize,
    ) {
        match option {
            Some((_, cursor)) => assert_eq!(cursor.get_position(), expected),
            None => panic!("expected position {}", expected),
        }
    }

    // #[test]
    // fn matches_any() {
    //     let input = Stream::<char>::from_str("abc");
    //     let result = parse(matcher::any(), input);

    //     assert_result(&result, &'a');
    //     assert_position(&result, 1);
    // }

    // #[test]
    // fn matches_eof() {
    //     let mut input = Stream::<char>::from_str("");
    //     let result = parse(matcher::eof(&true), &mut input);

    //     assert_result(&result, &true);
    //     assert_position(&result, 0);
    // }
}
