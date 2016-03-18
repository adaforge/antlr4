#pragma once

/*
 * [The "BSD license"]
 *  Copyright (c) 2013 Dan McLaughlin
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 *  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 *  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// Defines for the Guid class.
#ifdef _WIN32
  #define GUID_WINDOWS
#elseif __APPLE__
  #define GUID_CFUUID
#else
  #define GUID_LIBUUID
#endif

// This can be regenerated from the command line by using
// ls *.h | sed "s/\.h\*/;/" | sed 's/^/class /'

// Autogenerated
class XPathLexer;
class XPathParser;

namespace org {
    namespace antlr {
        namespace v4 {
            namespace runtime {
                class IllegalStateException;
                class IllegalArgumentException;
                class NoSuchElementException;
                class NullPointerException;
                class InputMismatchException;
                class ParseCancellationException;
                class InputMismatchException;
                class EmptyStackException;
                class LexerNoViableAltException;
                
                class ANTLRErrorListener;
                class ANTLRErrorStrategy;
                class ANTLRFileStream;
                class ANTLRInputStream;
                class BailErrorStrategy;
                class BaseErrorListener;
                class BufferedTokenStream;
                class CharStream;
                class CommonToken;
                class CommonTokenFactory;
                class CommonTokenStream;
                class ConsoleErrorListener;
                class DefaultErrorStrategy;
                class DiagnosticErrorListener;
                class FailedPredicateException;
                class InputMismatchException;
                class IntStream;
                class InterpreterRuleContext;
                class IErrorListener;
                template<typename Symbol, typename ATNInterpreter> class IRecognizer;
                class Lexer;
                class LexerInterpreter;
                class LexerNoViableAltException;
                class ListTokenSource;
                class NoViableAltException;
                class Parser;
                class ParserInterpreter;
                class ParserRuleContext;
                class ProxyErrorListener;
                class RecognitionException;
                template<typename T1, typename T2> class Recognizer;
                class RuleContext;
                class Token;
                template<typename Symbol> class TokenFactory;
                class TokenSource;
                class TokenStream;
                class TokenStreamRewriter;
                class UnbufferedCharStream;
                template<typename T> class UnbufferedTokenStream;
                class WritableToken;
                
                namespace misc {
                    template<typename T> class AbstractEqualityComparator;
                    template<typename T> class Array2DHashSet;
                    template<typename Key1, typename Key2, typename Value> class DoubleKeyMap;
                    template<typename T> class EqualityComparator;
                    
                    class GraphicsSupport;
                    class IntSet;
                    class Interval;
                    class IntervalSet;
                    class JFileChooserConfirmOverwrite;
                    class LogManager;
                    template<typename K, typename V> class MultiMap;
                    class MurmurHash;
                    class NotNull;
                    class ObjectEqualityComparator;
                    template<typename T> class OrderedHashSet;
                    class ParseCancellationException;
                    class TestRig;
                    class Utils;
                }
                namespace atn {
                    class ATN;
                    class ATNConfig;
                    class ATNConfigSet;
                    class ATNDeserializationOptions;
                    class ATNDeserializer;
                    class ATNSerializer;
                    class ATNSimulator;
                    class ATNState;
                    enum class ATNType;
                    class AbstractPredicateTransition;
                    class ActionTransition;
                    class ArrayPredictionContext;
                    class AtomTransition;
                    class BasicBlockStartState;
                    class BasicState;
                    class BlockEndState;
                    class BlockStartState;
                    class DecisionState;
                    class EmptyPredictionContext;
                    class EpsilonTransition;
                    class LL1Analyzer;
                    class LexerATNConfig;
                    class LexerATNSimulator;
                    class LoopEndState;
                    class NotSetTransition;
                    class OrderedATNConfigSet;
                    class ParserATNSimulator;
                    class PlusBlockStartState;
                    class PlusLoopbackState;
                    class PrecedencePredicateTransition;
                    class PredicateTransition;
                    class PredictionContext;
                    class PredictionContextCache;
                    enum class PredictionMode;
					class PredictionModeClass;
                    class RangeTransition;
                    class RuleStartState;
                    class RuleStopState;
                    class RuleTransition;
                    class SemanticContext;
                    class SetTransition;
                    class SingletonPredictionContext;
                    class StarBlockStartState;
                    class StarLoopEntryState;
                    class StarLoopbackState;
                    class TokensStartState;
                    class Transition;
                    class WildcardTransition;
                }
                namespace dfa {
                    class DFA;
                    class DFASerializer;
                    class DFAState;
                    class LexerDFASerializer;
                }
                namespace tree {
                    template <typename t> class AbstractParseTreeVisitor;
                    class ErrorNode;
                    class ErrorNodeImpl;
                    class ParseTree;
                    class ParseTreeListener;
                    template<typename T> class ParseTreeProperty;
                    template<typename T> class ParseTreeVisitor;
                    class ParseTreeWalker;
                    class RuleNode;
                    class SyntaxTree;
                    class TerminalNode;
                    class TerminalNodeImpl;
                    class Tree;
                    class Trees;
                    
                    namespace pattern {
                        class Chunk;
                        class ParseTreeMatch;
                        class ParseTreePattern;
                        class ParseTreePatternMatcher;
                        class RuleTagToken;
                        class TagChunk;
                        class TextChunk;
                        class TokenTagToken;
                    }
                    
                    namespace xpath {
                        class XPath;
                        class XPathElement;
                        class XPathLexerErrorListener;
                        class XPathRuleAnywhereElement;
                        class XPathRuleElement;
                        class XPathTokenAnywhereElement;
                        class XPathTokenElement;
                        class XPathWildcardAnywhereElement;
                        class XPathWildcardElement;
                        
                    }
                }
            }
        }
    }
}
