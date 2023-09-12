mod declaration;
mod expression;
mod ksx;
mod reference;
mod type_expression;
pub mod weak;
use crate::parser::{
    declaration::{Declaration, DeclarationNode},
    expression::{ksx::KSX, Expression},
    module::{Module, ModuleNode},
    position::Decrement,
    types::type_expression::TypeExpression,
};
use combine::Stream;
use std::{collections::HashMap, fmt::Debug};

use self::reference::ToRef;

pub trait Register<T>: Sized {
    fn register(self, ctx: &mut Context) -> T;
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

    x.register(&mut ctx)
}

fn identify_module<T>(x: ModuleNode<T, ()>, ctx: &mut Context) -> Module<DeclarationNode<T, usize>>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    let declarations =
        x.0.declarations
            .into_iter()
            .map(|x| x.register(ctx))
            .collect::<Vec<_>>();

    Module {
        imports: x.0.imports,
        declarations,
    }
}

impl<T> Register<ModuleNode<T, usize>> for ModuleNode<T, ()>
where
    T: Stream<Token = char>,
    T::Position: Copy + Debug + Decrement,
{
    fn register(self, ctx: &mut Context) -> ModuleNode<T, usize> {
        let module = identify_module(self, ctx);
        let fragment = Fragment::Module(module.to_ref());
        let id = ctx.register(fragment);

        ModuleNode(module, id)
    }
}

#[cfg(test)]
mod tests {
    use super::Context;
    use crate::{
        analyzer::Register,
        parser::{
            declaration::{
                storage::{Storage, Visibility},
                Declaration,
            },
            expression::{primitive::Primitive, Expression},
            module::{
                import::{Import, Source, Target},
                Module, ModuleNode,
            },
            types::type_expression::TypeExpression,
        },
        test::fixture as f,
    };

    #[test]
    fn module() {
        let ctx = &mut Context::new();

        let result = ModuleNode(
            Module {
                imports: vec![Import {
                    source: Source::Root,
                    path: vec![String::from("bar"), String::from("fizz")],
                    aliases: Some(vec![(Target::Module, Some(String::from("Fizz")))]),
                }],
                declarations: vec![f::dc(
                    Declaration::Constant {
                        name: Storage(Visibility::Public, String::from("BUZZ")),
                        value_type: Some(f::txc(TypeExpression::Nil, ())),
                        value: f::xc(Expression::Primitive(Primitive::Nil), ()),
                    },
                    (),
                )],
            },
            (),
        )
        .register(ctx);

        assert_eq!(
            result,
            ModuleNode(
                Module {
                    imports: vec![Import {
                        source: Source::Root,
                        path: vec![String::from("bar"), String::from("fizz")],
                        aliases: Some(vec![(Target::Module, Some(String::from("Fizz")))]),
                    }],
                    declarations: vec![f::dc(
                        Declaration::Constant {
                            name: Storage(Visibility::Public, String::from("BUZZ")),
                            value_type: Some(f::txc(TypeExpression::Nil, 0,)),
                            value: f::xc(Expression::Primitive(Primitive::Nil), 1,),
                        },
                        2,
                    )],
                },
                3,
            )
        )
    }
}
