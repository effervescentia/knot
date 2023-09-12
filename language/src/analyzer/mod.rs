mod declaration;
mod expression;
mod ksx;
mod module;
mod type_expression;
pub mod weak;
use crate::parser::{
    declaration::Declaration,
    expression::{ksx::KSX, Expression},
    module::{Module, ModuleNode},
    position::Decrement,
    types::type_expression::TypeExpression,
};
use combine::Stream;
use std::{collections::HashMap, fmt::Debug};

pub trait Analyze<Result, Ref>: Sized {
    type Value<C>;

    fn register(self, ctx: &mut Context) -> Result;

    fn identify(value: Self::Value<()>, ctx: &mut Context) -> Self::Value<usize>;

    fn to_ref<'a>(value: &'a Self::Value<usize>) -> Ref;
}

#[derive(Debug, Clone, PartialEq)]
pub enum Type<T> {
    Nil,
    Boolean,
    Integer,
    Float,
    String,
    Style,
    Element,
    Enumerated(Vec<(String, Vec<T>)>),
    Function(Vec<T>, T),
    View(Vec<T>, T),
    Module(Vec<(String, T)>),
}

#[derive(Debug, PartialEq)]
pub enum Fragment {
    Expression(Expression<usize, usize>),
    KSX(KSX<usize, usize>),
    TypeExpression(TypeExpression<usize>),
    Declaration(Declaration<usize, usize, usize>),
    Module(Module<usize>),
}

pub struct Context {
    next_id: usize,
    fragments: HashMap<usize, Fragment>,
}

impl Context {
    pub fn new() -> Self {
        Self {
            next_id: 0,
            fragments: HashMap::new(),
        }
    }

    pub fn generate_id(&mut self) -> usize {
        let id = self.next_id;
        self.next_id += 1;
        id
    }

    pub fn register(&mut self, x: Fragment) -> usize {
        let id = self.generate_id();
        self.fragments.insert(id, x);
        id
    }
}

pub fn analyze<T>(x: ModuleNode<T, ()>) -> ModuleNode<T, usize>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    let mut ctx = Context::new();

    let result = x.register(&mut ctx);

    result
}
