mod shape;

use crate::{
    ast::TypePrimitive,
    format::{Lambda, Object},
};
pub use shape::{ToShape, Type as Shape};
use std::fmt::{Debug, Display, Pointer};

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum Kind {
    Type,
    Value,
    Mixed,
}

impl Kind {
    pub const fn invert(&self) -> Self {
        match self {
            Self::Type => Self::Value,
            Self::Value => Self::Type,
            Self::Mixed => Self::Mixed,
        }
    }

    pub fn can_accept(&self, other: &Self) -> bool {
        self == other || matches!((self, other), (Self::Mixed, _) | (_, Self::Mixed))
    }
}

impl Display for Kind {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        match self {
            Self::Type => write!(f, "type"),
            Self::Value => write!(f, "value"),
            Self::Mixed => write!(f, "mixed"),
        }
    }
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub enum Enumerated<T> {
    Declaration(Vec<(String, Vec<T>)>),
    Variant(Vec<T>, T),
    Instance(T),
}

impl<T> Enumerated<T> {
    pub fn map<R, F>(&self, f: &F) -> Enumerated<R>
    where
        F: Fn(&T) -> R,
    {
        match self {
            Self::Declaration(variants) => Enumerated::Declaration(
                variants
                    .iter()
                    .map(|(name, parameters)| (name.clone(), parameters.iter().map(f).collect()))
                    .collect(),
            ),

            Self::Variant(parameters, instance) => {
                Enumerated::Variant(parameters.iter().map(f).collect(), f(instance))
            }

            Self::Instance(x) => Enumerated::Instance(f(x)),
        }
    }

    pub fn opt_map<R, F>(&self, f: &F) -> Option<Enumerated<R>>
    where
        F: Fn(&T) -> Option<R>,
    {
        match self {
            Self::Declaration(variants) => Some(Enumerated::Declaration(
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

            Self::Variant(parameters, instance) => Some(Enumerated::Variant(
                parameters.iter().map(f).collect::<Option<Vec<_>>>()?,
                f(instance)?,
            )),

            Self::Instance(x) => Some(Enumerated::Instance(f(x)?)),
        }
    }

    pub fn to_shape(&self) -> Enumerated<()> {
        self.map(&|_| ())
    }
}

impl<T> Display for Enumerated<T> {
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        f.write_str("enum")
    }
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub enum ObjectTypeEntry<T> {
    Required(String, T),
    Optional(String, T),
}

impl<T> ObjectTypeEntry<T> {
    pub fn name(&self) -> &str {
        match self {
            Self::Required(name, _) | Self::Optional(name, _) => name,
        }
    }

    pub const fn value(&self) -> &T {
        match self {
            Self::Required(_, value) | Self::Optional(_, value) => value,
        }
    }

    pub const fn is_required(&self) -> bool {
        matches!(self, Self::Required(..))
    }

    pub fn map<R, F>(&self, f: F) -> ObjectTypeEntry<R>
    where
        F: FnOnce(&T) -> R,
    {
        match self {
            Self::Required(name, x) => ObjectTypeEntry::Required(name.clone(), f(x)),

            Self::Optional(name, x) => ObjectTypeEntry::Optional(name.clone(), f(x)),
        }
    }

    pub fn opt_map<R, F>(&self, f: F) -> Option<ObjectTypeEntry<R>>
    where
        F: FnOnce(&T) -> Option<R>,
    {
        Some(match self {
            Self::Required(name, x) => ObjectTypeEntry::Required(name.clone(), f(x)?),

            Self::Optional(name, x) => ObjectTypeEntry::Optional(name.clone(), f(x)?),
        })
    }
}

impl<T> Display for ObjectTypeEntry<T>
where
    T: Display,
{
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        match self {
            Self::Required(binding, x) => write!(f, "{binding}: {x}"),

            Self::Optional(binding, x) => write!(f, "{binding}?: {x}"),
        }
    }
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub enum Type<T> {
    Nil,
    Boolean,
    Integer,
    Float,
    String,
    Style,
    Element,

    Enumerated(Enumerated<T>),
    Function(Vec<T>, T),
    Object(Vec<ObjectTypeEntry<T>>),
    View(Vec<ObjectTypeEntry<T>>),
    Module(Vec<(String, Kind, T)>),
}

impl<T> Type<T> {
    pub fn map<R, F>(&self, f: &F) -> Type<R>
    where
        F: Fn(&T) -> R,
    {
        match self {
            Self::Nil => Type::Nil,
            Self::Boolean => Type::Boolean,
            Self::Integer => Type::Integer,
            Self::Float => Type::Float,
            Self::String => Type::String,
            Self::Style => Type::Style,
            Self::Element => Type::Element,

            Self::Enumerated(x) => Type::Enumerated(x.map(f)),

            Self::Function(parameters, result) => {
                Type::Function(parameters.iter().map(f).collect(), f(result))
            }

            Self::Object(entries) => {
                Type::Object(entries.iter().map(|entry| entry.map(f)).collect())
            }

            Self::View(parameters) => Type::View(
                parameters
                    .iter()
                    .map(|parameter| parameter.map(f))
                    .collect(),
            ),

            Self::Module(declarations) => Type::Module(
                declarations
                    .iter()
                    .map(|(name, kind, x)| (name.clone(), *kind, f(x)))
                    .collect(),
            ),
        }
    }

    pub fn opt_map<R, F>(&self, f: &F) -> Option<Type<R>>
    where
        F: Fn(&T) -> Option<R>,
    {
        match self {
            Self::Nil => Some(Type::Nil),
            Self::Boolean => Some(Type::Boolean),
            Self::Integer => Some(Type::Integer),
            Self::Float => Some(Type::Float),
            Self::String => Some(Type::String),
            Self::Style => Some(Type::Style),
            Self::Element => Some(Type::Element),

            Self::Enumerated(x) => Some(Type::Enumerated(x.opt_map(f)?)),

            Self::Function(parameters, result) => Some(Type::Function(
                parameters.iter().map(f).collect::<Option<Vec<_>>>()?,
                f(result)?,
            )),

            Self::Object(entries) => Some(Type::Object(
                entries
                    .iter()
                    .map(|entry| entry.opt_map(f))
                    .collect::<Option<Vec<_>>>()?,
            )),

            Self::View(parameters) => Some(Type::View(
                parameters
                    .iter()
                    .map(|parameter| parameter.opt_map(f))
                    .collect::<Option<Vec<_>>>()?,
            )),

            Self::Module(declarations) => Some(Type::Module(
                declarations
                    .iter()
                    .map(|(name, kind, x)| Some((name.clone(), *kind, f(x)?)))
                    .collect::<Option<Vec<_>>>()?,
            )),
        }
    }

    pub fn to_shallow(&self) -> Type<()> {
        self.map(&|_| ())
    }
}

impl<T> Display for Type<T>
where
    T: Display,
{
    fn fmt(&self, f: &mut std::fmt::Formatter) -> std::fmt::Result {
        match self {
            Self::Nil => Display::fmt(&TypePrimitive::Nil, f),
            Self::Boolean => Display::fmt(&TypePrimitive::Boolean, f),
            Self::Integer => Display::fmt(&TypePrimitive::Integer, f),
            Self::Float => Display::fmt(&TypePrimitive::Float, f),
            Self::String => Display::fmt(&TypePrimitive::String, f),
            Self::Style => Display::fmt(&TypePrimitive::Style, f),
            Self::Element => Display::fmt(&TypePrimitive::Element, f),

            Self::Enumerated(enumerated) => enumerated.fmt(f),

            Self::Function(parameters, result) => Lambda(parameters, result).fmt(f),

            Self::Object(entries) => Object(entries).fmt(f),

            Self::Module(entities) => write!(
                f,
                "module {}",
                Object(
                    entities
                        .iter()
                        .map(|(name, _, type_)| format!("{name}: {type_}"))
                )
            ),

            Self::View(attributes) => write!(f, "view {{ {} }}", Object(attributes)),
        }
    }
}
