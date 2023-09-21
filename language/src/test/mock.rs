use crate::common::position::Decrement;
use combine::{parser::char as p, Parser, Stream};
use std::fmt::Debug;

pub const MOCK_TOKEN: &str = "__mock__";

#[derive(Debug, PartialEq)]
pub struct MockResult;

pub fn mock<T>() -> impl Parser<T, Output = MockResult>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    p::string(MOCK_TOKEN).map(|_| MockResult)
}
