use crate::matcher;
use combine::{Parser, Stream};
use std::fmt::Debug;

pub const MOCK_TOKEN: &str = "__mock__";

#[derive(Debug, PartialEq)]
pub struct MockResult;

pub fn mock<T>() -> impl Parser<T, Output = MockResult>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
{
    matcher::keyword(MOCK_TOKEN).map(|_| MockResult)
}
