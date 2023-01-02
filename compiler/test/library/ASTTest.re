open Kore;

module A = AST.Result;
module U = Util.ResultUtil;

let dump =
  Language.Program.program_to_xml(~@AST.Type.pp)
  % ~@Pretty.XML.xml(Fmt.string);

let rec get_xml_depth =
  fun
  | Pretty.XML.Node(name, attrs, children) =>
    1
    + (children |> List.map(get_xml_depth) |> List.fold_left(Stdlib.max, 0));

let suite =
  "Library.AST"
  >::: [
    "Dump.pp() - nil primitive"
    >: (
      () =>
        Assert.string(
          "<Program>
  <ModuleStatement range=0.0>
    <Declaration>
      <Named range=0.0 value=nil_const />
      <Entity range=0.0 type=nil>
        <Constant>
          <Expression range=0.0 type=nil>
            <Nil />
          </Expression>
        </Constant>
      </Entity>
    </Declaration>
  </ModuleStatement>
</Program>",
          dump(Fixtures.nil_const),
        )
    ),
    "Dump.pp() - integer primitive"
    >: (
      () =>
        Assert.string(
          "<Program>
  <ModuleStatement range=0.0>
    <Declaration>
      <Named range=0.0 value=int_const />
      <Entity range=0.0 type=integer>
        <Constant>
          <Expression range=0.0 type=integer>
            <Integer value=123 />
          </Expression>
        </Constant>
      </Entity>
    </Declaration>
  </ModuleStatement>
</Program>",
          dump(Fixtures.int_const),
        )
    ),
    "Dump.pp() - float primitive"
    >: (
      () =>
        Assert.string(
          "<Program>
  <ModuleStatement range=0.0>
    <Declaration>
      <Named range=0.0 value=float_const />
      <Entity range=0.0 type=float>
        <Constant>
          <Expression range=0.0 type=float>
            <Float value=123.000 />
          </Expression>
        </Constant>
      </Entity>
    </Declaration>
  </ModuleStatement>
</Program>",
          dump(Fixtures.float_const),
        )
    ),
    "Dump.pp() - boolean primitive"
    >: (
      () =>
        Assert.string(
          "<Program>
  <ModuleStatement range=0.0>
    <Declaration>
      <Named range=0.0 value=bool_const />
      <Entity range=0.0 type=boolean>
        <Constant>
          <Expression range=0.0 type=boolean>
            <Boolean value=true />
          </Expression>
        </Constant>
      </Entity>
    </Declaration>
  </ModuleStatement>
</Program>",
          dump(Fixtures.bool_const),
        )
    ),
    "Dump.pp() - string primitive"
    >: (
      () =>
        Assert.string(
          "<Program>
  <ModuleStatement range=0.0>
    <Declaration>
      <Named range=0.0 value=string_const />
      <Entity range=0.0 type=string>
        <Constant>
          <Expression range=0.0 type=string>
            <String value=\"foo\" />
          </Expression>
        </Constant>
      </Entity>
    </Declaration>
  </ModuleStatement>
</Program>",
          dump(Fixtures.string_const),
        )
    ),
    "Dump.pp() - identifier expression"
    >: (
      () =>
        Assert.string(
          "<Program>
  <ModuleStatement range=0.0>
    <Declaration>
      <Named range=0.0 value=identifier_const />
      <Entity range=0.0 type=integer>
        <Constant>
          <Expression range=0.0 type=integer>
            <Identifier name=foo />
          </Expression>
        </Constant>
      </Entity>
    </Declaration>
  </ModuleStatement>
</Program>",
          dump(Fixtures.identifier_const),
        )
    ),
    "Dump.pp() - JSX expression"
    >: (
      () =>
        Assert.string(
          "<Program>
  <ModuleStatement range=0.0>
    <Declaration>
      <Named range=0.0 value=jsx_const />
      <Entity range=0.0 type=element>
        <Constant>
          <Expression range=0.0 type=element>
            <KSX>
              <Element>
                <Name range=0.0 type=View<(), nil> value=Foo />
              </Element>
            </KSX>
          </Expression>
        </Constant>
      </Entity>
    </Declaration>
  </ModuleStatement>
</Program>",
          dump(Fixtures.jsx_const),
        )
    ),
    "Dump.pp() - group expression"
    >: (
      () =>
        Assert.string(
          "<Program>
  <ModuleStatement range=0.0>
    <Declaration>
      <Named range=0.0 value=group_const />
      <Entity range=0.0 type=integer>
        <Constant>
          <Expression range=0.0 type=integer>
            <Group>
              <Expression range=0.0 type=integer>
                <Integer value=123 />
              </Expression>
            </Group>
          </Expression>
        </Constant>
      </Entity>
    </Declaration>
  </ModuleStatement>
</Program>",
          dump(Fixtures.group_const),
        )
    ),
    "Dump.pp() - closure expression"
    >: (
      () =>
        Assert.string(
          "<Program>
  <ModuleStatement range=0.0>
    <Declaration>
      <Named range=0.0 value=closure_const />
      <Entity range=0.0 type=boolean>
        <Constant>
          <Expression range=0.0 type=boolean>
            <Closure>
              <Statement range=0.0 type=nil>
                <Variable>
                  <Name range=0.0 value=foo />
                  <Value>
                    <Expression range=0.0 type=integer>
                      <Integer value=123 />
                    </Expression>
                  </Value>
                </Variable>
              </Statement>
              <Statement range=0.0 type=nil>
                <Variable>
                  <Name range=0.0 value=bar />
                  <Value>
                    <Expression range=0.0 type=nil>
                      <Nil />
                    </Expression>
                  </Value>
                </Variable>
              </Statement>
              <Statement range=0.0 type=boolean>
                <Effect>
                  <Expression range=0.0 type=boolean>
                    <Boolean value=false />
                  </Expression>
                </Effect>
              </Statement>
            </Closure>
          </Expression>
        </Constant>
      </Entity>
    </Declaration>
  </ModuleStatement>
</Program>",
          dump(Fixtures.closure_const),
        )
    ),
    "Dump.pp() - binary operation"
    >: (
      () =>
        Assert.string(
          "<Program>
  <ModuleStatement range=0.0>
    <Declaration>
      <Named range=0.0 value=and_bool_const />
      <Entity range=0.0 type=boolean>
        <Constant>
          <Expression range=0.0 type=boolean>
            <And>
              <Left>
                <Expression range=0.0 type=boolean>
                  <Boolean value=true />
                </Expression>
              </Left>
              <Right>
                <Expression range=0.0 type=boolean>
                  <Boolean value=false />
                </Expression>
              </Right>
            </And>
          </Expression>
        </Constant>
      </Entity>
    </Declaration>
  </ModuleStatement>
</Program>",
          dump(Fixtures.and_bool_const),
        )
    ),
    "Dump.pp() - unary operation"
    >: (
      () =>
        Assert.string(
          "<Program>
  <ModuleStatement range=0.0>
    <Declaration>
      <Named range=0.0 value=negative_int_const />
      <Entity range=0.0 type=integer>
        <Constant>
          <Expression range=0.0 type=integer>
            <Negative>
              <Expression range=0.0 type=integer>
                <Integer value=123 />
              </Expression>
            </Negative>
          </Expression>
        </Constant>
      </Entity>
    </Declaration>
  </ModuleStatement>
</Program>",
          dump(Fixtures.negative_int_const),
        )
    ),
    "Dump.pp() - jsx"
    >: (
      () =>
        Assert.string(
          "<Program>
  <ModuleStatement range=0.0>
    <Declaration>
      <Named range=0.0 value=complex_jsx_const />
      <Entity range=0.0 type=element>
        <Constant>
          <Expression range=0.0 type=element>
            <KSX>
              <Element>
                <Name range=0.0 type=View<(), nil> value=Foo />
                <Attribute range=0.0>
                  <Property>
                    <Name range=0.0 value=buzz />
                  </Property>
                </Attribute>
                <Child range=0.0>
                  <Node>
                    <KSX>
                      <Element>
                        <Name range=0.0 type=View<(), nil> value=Bar />
                      </Element>
                    </KSX>
                  </Node>
                </Child>
                <Child range=0.0>
                  <InlineExpression>
                    <Expression range=0.0 type=nil>
                      <Nil />
                    </Expression>
                  </InlineExpression>
                </Child>
                <Child range=0.0>
                  <Text value=fizzbuzz />
                </Child>
                <Child range=0.0>
                  <Node>
                    <KSX>
                      <Fragment />
                    </KSX>
                  </Node>
                </Child>
              </Element>
            </KSX>
          </Expression>
        </Constant>
      </Entity>
    </Declaration>
  </ModuleStatement>
</Program>",
          dump(Fixtures.complex_jsx_const),
        )
    ),
    "Dump.pp() - inline function"
    >: (
      () =>
        Assert.string(
          "<Program>
  <ModuleStatement range=0.0>
    <Declaration>
      <Named range=0.0 value=inline_function />
      <Entity range=0.0 type=(integer, integer) -> integer>
        <Function>
          <Parameter range=0.0 type=integer>
            <Name range=0.0 value=foo />
          </Parameter>
          <Parameter range=0.0 type=integer>
            <Name range=0.0 value=bar />
            <Default>
              <Expression range=0.0 type=integer>
                <Integer value=3 />
              </Expression>
            </Default>
          </Parameter>
          <Body>
            <Expression range=0.0 type=integer>
              <Add>
                <Left>
                  <Expression range=0.0 type=integer>
                    <Identifier name=foo />
                  </Expression>
                </Left>
                <Right>
                  <Expression range=0.0 type=integer>
                    <Identifier name=bar />
                  </Expression>
                </Right>
              </Add>
            </Expression>
          </Body>
        </Function>
      </Entity>
    </Declaration>
  </ModuleStatement>
</Program>",
          dump(Fixtures.inline_function),
        )
    ),
    "Dump.pp() - multiline function"
    >: (
      () =>
        Assert.string(
          "<Program>
  <ModuleStatement range=0.0>
    <Declaration>
      <Named range=0.0 value=multiline_function />
      <Entity range=0.0 type=(integer, integer) -> integer>
        <Function>
          <Body>
            <Expression range=0.0 type=integer>
              <Closure>
                <Statement range=0.0 type=nil>
                  <Variable>
                    <Name range=0.0 value=zip />
                    <Value>
                      <Expression range=0.0 type=integer>
                        <Integer value=3 />
                      </Expression>
                    </Value>
                  </Variable>
                </Statement>
                <Statement range=0.0 type=nil>
                  <Variable>
                    <Name range=0.0 value=zap />
                    <Value>
                      <Expression range=0.0 type=integer>
                        <Integer value=4 />
                      </Expression>
                    </Value>
                  </Variable>
                </Statement>
                <Statement range=0.0 type=integer>
                  <Effect>
                    <Expression range=0.0 type=integer>
                      <Mult>
                        <Left>
                          <Expression range=0.0 type=integer>
                            <Identifier name=zip />
                          </Expression>
                        </Left>
                        <Right>
                          <Expression range=0.0 type=integer>
                            <Identifier name=zap />
                          </Expression>
                        </Right>
                      </Mult>
                    </Expression>
                  </Effect>
                </Statement>
              </Closure>
            </Expression>
          </Body>
        </Function>
      </Entity>
    </Declaration>
  </ModuleStatement>
</Program>",
          dump(Fixtures.multiline_function),
        )
    ),
    "Dump.pp() - main import"
    >: (
      () =>
        Assert.string(
          "<Program>
  <ModuleStatement range=0.0>
    <Import namespace=main_import>
      <MainImport range=0.0 value=Foo />
    </Import>
  </ModuleStatement>
</Program>",
          dump(Fixtures.main_import),
        )
    ),
    "Dump.pp() - named import"
    >: (
      () =>
        Assert.string(
          "<Program>
  <ModuleStatement range=0.0>
    <Import namespace=named_import>
      <NamedImport range=0.0>
        <Name range=0.0 value=foo />
      </NamedImport>
    </Import>
  </ModuleStatement>
</Program>",
          dump(Fixtures.named_import),
        )
    ),
    "Dump.pp() - program" >: (() => Assert.string("<Program />", dump([]))),
  ];
