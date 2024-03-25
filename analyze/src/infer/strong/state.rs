use super::data::{Data, Output, Result, Strong};
use crate::{
    error::ResolveError,
    infer::{weak, BindingMap, NodeDescriptor},
    typed::ReferenceType,
};
use kore::invariant;
use lang::{
    types::{self, Enumerated, Kind, Type},
    FragmentMap, NodeId,
};
use std::{cell::OnceCell, collections::BTreeMap, rc::Rc};

/// type resolved from the `State` during inference
type Resolved<'a> = std::result::Result<&'a types::Type<NodeId>, &'a ResolveError>;

type Warning<'a> = (&'a NodeDescriptor<'a>, String);

pub struct State<'a> {
    pub fragments: &'a FragmentMap,

    pub bindings: BindingMap,

    pub nodes: Vec<NodeDescriptor<'a>>,

    pub types: BTreeMap<NodeId, Strong>,

    pub warnings: Vec<Warning<'a>>,
}

impl<'a> State<'a> {
    pub fn from_weak(mut weak: weak::Result<'a>) -> Self {
        let nodes = weak.build_descriptors();

        Self {
            fragments: weak.fragments,
            bindings: weak.bindings,
            nodes,
            types: Default::default(),
            warnings: Default::default(),
        }
    }

    pub fn next(prev: Self) -> (Vec<NodeDescriptor<'a>>, Self) {
        (
            prev.nodes,
            Self {
                nodes: Default::default(),
                ..prev
            },
        )
    }

    pub fn is_done(&self) -> bool {
        self.nodes.is_empty()
    }

    pub fn get_type(
        &self,
        id: &NodeId,
        allowed_kind: &Kind,
    ) -> Option<&std::result::Result<Data, ResolveError>> {
        self.types.get(id).and_then(|(kind, strong)| {
            if !allowed_kind.can_accept(kind) {
                return None;
            }

            Some(strong)
        })
    }

    pub fn resolve(&self, id: &NodeId, allowed_kind: &Kind) -> Option<Resolved> {
        self.get_type(id, allowed_kind)
            .and_then(|strong| match strong {
                Ok(Data::Local(local)) => Some(Ok(local)),

                Ok(Data::Inherit(from_id)) => self.resolve_any(from_id),

                Err(err) => Some(Err(err)),
            })
    }

    pub fn resolve_value(&self, id: &NodeId) -> Option<Resolved> {
        self.resolve(id, &Kind::Value)
    }

    pub fn resolve_any(&self, id: &NodeId) -> Option<Resolved> {
        self.resolve(id, &Kind::Mixed)
    }

    fn finalize_type(x: Type<NodeId>, output: &Output) -> Rc<ReferenceType> {
        let get_type = |id| {
            Rc::clone(
                output
                    .types
                    .get(id)
                    .and_then(OnceCell::get)
                    .unwrap_or_else(|| invariant!("type not found")),
            )
        };

        match x {
            Type::Nil => Rc::new(ReferenceType(Type::Nil)),
            Type::Boolean => Rc::new(ReferenceType(Type::Boolean)),
            Type::Integer => Rc::new(ReferenceType(Type::Integer)),
            Type::Float => Rc::new(ReferenceType(Type::Float)),
            Type::String => Rc::new(ReferenceType(Type::String)),
            Type::Style => Rc::new(ReferenceType(Type::Style)),
            Type::Element => Rc::new(ReferenceType(Type::Element)),

            Type::Enumerated(x) => Rc::new(ReferenceType(Type::Enumerated(match x {
                Enumerated::Declaration(variants) => Enumerated::Declaration(
                    variants
                        .iter()
                        .map(|(name, xs)| (name.clone(), xs.iter().map(get_type).collect()))
                        .collect(),
                ),

                Enumerated::Variant(parameters, instance) => Enumerated::Variant(
                    parameters.iter().map(get_type).collect(),
                    get_type(&instance),
                ),

                Enumerated::Instance(x) => Enumerated::Instance(get_type(&x)),
            }))),

            Type::Function(parameters, x) => Rc::new(ReferenceType(Type::Function(
                parameters.iter().map(get_type).collect(),
                get_type(&x),
            ))),

            Type::View(parameters) => Rc::new(ReferenceType(Type::View(
                parameters.iter().map(get_type).collect(),
            ))),

            Type::Module(declarations) => Rc::new(ReferenceType(Type::Module(
                declarations
                    .iter()
                    .map(|(name, kind, x)| (name.clone(), *kind, get_type(x)))
                    .collect(),
            ))),
        }
    }

    pub fn into_result(self) -> Result {
        let output = Output::new(self.types.keys());
        let mut errors = vec![];

        let get_cell = |id| {
            output
                .types
                .get(&id)
                .unwrap_or_else(|| invariant!("no cell exists to store type"))
        };

        for (id, (_, x)) in self.types {
            match x {
                Ok(Data::Local(x)) => {
                    let cell = get_cell(id);

                    cell.set(State::finalize_type(x, &output)).ok();
                }

                Ok(Data::Inherit(from_id)) => {
                    let cell = get_cell(id);
                    let from_cell = get_cell(from_id);

                    cell.set(Rc::clone(
                        from_cell
                            .get()
                            .unwrap_or_else(|| invariant!("inherited cell is empty")),
                    ))
                    .ok();
                }

                Err(err) => errors.push((id, err)),
            }
        }

        if errors.is_empty() {
            Ok(output)
        } else {
            Err(errors)
        }
    }
}
