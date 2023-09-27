use super::{
    infer::strong::{SemanticError, Strong},
    FinalType, PreviewType, RefKind,
};
use std::fmt::Debug;

#[derive(Clone, Debug, PartialEq)]
pub enum Type<T> {
    Nil,
    Boolean,
    Integer,
    Float,
    String,
    Style,
    Element,

    Enumerated(Vec<(String, Vec<T>)>),
    EnumeratedVariant(Vec<T>, T),
    EnumeratedInstance(T),

    Function(Vec<T>, T),
    View(Vec<T>),
    Module(Vec<(String, RefKind, T)>),
}

impl<T> Type<T> {
    pub fn opt_map<R>(&self, f: &impl Fn(&T) -> Option<R>) -> Option<Type<R>> {
        match self {
            Self::Nil => Some(Type::Nil),
            Self::Boolean => Some(Type::Boolean),
            Self::Integer => Some(Type::Integer),
            Self::Float => Some(Type::Float),
            Self::String => Some(Type::String),
            Self::Style => Some(Type::Style),
            Self::Element => Some(Type::Element),

            Self::Enumerated(variants) => Some(Type::Enumerated(
                variants
                    .iter()
                    .map(|(name, parameters)| {
                        Some((
                            name.clone(),
                            parameters.iter().map(f).collect::<Option<Vec<_>>>()?,
                        ))
                    })
                    .collect::<Option<Vec<_>>>()?,
            )),

            Self::EnumeratedVariant(parameters, x) => Some(Type::EnumeratedVariant(
                parameters.iter().map(f).collect::<Option<Vec<_>>>()?,
                f(x)?,
            )),

            Self::EnumeratedInstance(x) => Some(Type::EnumeratedInstance(f(x)?)),

            Self::Function(parameters, body) => Some(Type::Function(
                parameters.iter().map(f).collect::<Option<Vec<_>>>()?,
                f(body)?,
            )),

            Self::View(parameters) => Some(Type::View(
                parameters.iter().map(f).collect::<Option<Vec<_>>>()?,
            )),

            Self::Module(entries) => Some(Type::Module(
                entries
                    .iter()
                    .map(|(name, kind, x)| Some((name.clone(), kind.clone(), f(x)?)))
                    .collect::<Option<Vec<_>>>()?,
            )),
        }
    }
}

// impl Type<usize> {
//     pub fn preview(&self, get_strong: &impl Fn(&usize) -> Option<&Strong>) -> Option<PreviewType> {
//         self.opt_map(&|x| match get_strong(x) {
//             Some(Ok(typ)) => typ.preview(get_strong).map(Box::new),

//             _ => None,
//         })
//         .map(PreviewType)
//     }
// }
