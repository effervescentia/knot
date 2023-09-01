use super::{storage, Declaration, DeclarationRaw};
use crate::{matcher as m, types::type_expression};
use combine::{Parser, Stream};
use std::fmt::Debug;

pub fn type_alias<T>() -> impl Parser<T, Output = DeclarationRaw<T>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug,
{
    m::terminated((
        storage::storage("type"),
        m::symbol('='),
        type_expression::type_expression(),
    ))
    // TODO: concat range from type expression
    .map(|((name, start), _, value)| DeclarationRaw(Declaration::TypeAlias { name, value }, start))
}
