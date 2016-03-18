﻿#include "ATNDeserializer.h"
#include "ATNDeserializationOptions.h"
#include "Declarations.h"
#include "ATNState.h"
#include "ATN.h"
#include "LoopEndState.h"
#include "Token.h"
#include "BlockStartState.h"
#include "BlockEndState.h"

#include "ATNType.h"
#include "DecisionState.h"
#include "RuleStartState.h"
#include "RuleStopState.h"
#include "TokensStartState.h"
#include "RuleTransition.h"
#include "EpsilonTransition.h"
#include "PlusLoopbackState.h"
#include "PlusBlockStartState.h"
#include "StarLoopbackState.h"
#include "StarLoopEntryState.h"
#include "BasicBlockStartState.h"
#include "BasicState.h"
#include "AtomTransition.h"
#include "StarBlockStartState.h"
#include "RangeTransition.h"
#include "PredicateTransition.h"
#include "PrecedencePredicateTransition.h"
#include "ActionTransition.h"
#include "SetTransition.h"
#include "NotSetTransition.h"
#include "WildcardTransition.h"
#include "IntervalSet.h"

#include <exception>
#include <cstdint>
#include <utility>

/*
 * [The "BSD license"]
 *  Copyright (c) 2016 Mike Lischke
 *  Copyright (c) 2013 Terence Parr
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

using namespace antlrcpp;

namespace org {
    namespace antlr {
        namespace v4 {
            namespace runtime {
                namespace atn {
					const int ATNDeserializer::SERIALIZED_VERSION = 3;

                    /* This value should never change. Updates following this version are
                     * reflected as change in the unique ID SERIALIZED_UUID.
                     */
                    const Guid ATNDeserializer::BASE_SERIALIZED_UUID("33761B2D-78BB-4A43-8B0B-4F5BEE8AACF3");
                    const Guid ATNDeserializer::ADDED_PRECEDENCE_TRANSITIONS("1DA0C57D-6C06-438A-9B27-10BCB3CE0F61");
                  const Guid ATNDeserializer::ADDED_LEXER_ACTIONS("AADB8D7E-AEEF-4415-AD2B-8204D6CF042E");
                    const std::vector<Guid> ATNDeserializer::SUPPORTED_UUIDS = supportedUUIDsInitializer();
                    const Guid ATNDeserializer::SERIALIZED_UUID = ADDED_LEXER_ACTIONS;

                    ATNDeserializer::ATNDeserializer(): deserializationOptions(deserializationOptionsInitializer(nullptr)) {
                    }

                    ATNDeserializer::ATNDeserializer(ATNDeserializationOptions *dso): deserializationOptions(deserializationOptionsInitializer(dso)) {
                    }

                    bool ATNDeserializer::isFeatureSupported(const Guid &feature, const Guid &actualUuid) {
                      auto featureIterator = std::find(SUPPORTED_UUIDS.begin(), SUPPORTED_UUIDS.end(), feature);
                      if (featureIterator == SUPPORTED_UUIDS.end()) {
                        return false;
                      }
                      auto actualIterator = std::find(SUPPORTED_UUIDS.begin(), SUPPORTED_UUIDS.end(), actualUuid);
                      if (actualIterator == SUPPORTED_UUIDS.end()) {
                        return false;
                      }

                      return std::distance(featureIterator, actualIterator) >= 0;
                    }

                    ATN *ATNDeserializer::deserialize(const std::wstring& input) {
                        // Don't adjust the first value since that's the version number.
                      wchar_t data[input.size()];
                      data[0] = input[0];
                        for (size_t i = 1; i < input.size(); ++i) {
                            data[i] = input[i] - 2;
                        }

                        int p = 0;
                        int version = data[p++];
                        if (version != SERIALIZED_VERSION) {
                            std::wstring reason = L"Could not deserialize ATN with version" + std::to_wstring(version) + L"(expected " + std::to_wstring(SERIALIZED_VERSION) + L").";
                            
                            throw UnsupportedOperationException(reason);
                        }

                        Guid uuid = toUUID(data, p);
                      auto uuidIterator = std::find(SUPPORTED_UUIDS.begin(), SUPPORTED_UUIDS.end(), uuid);
                        p += 8;
                        if (uuidIterator == SUPPORTED_UUIDS.end()) {
                            std::wstring reason = L"Could not deserialize ATN with UUID " +
                            s2ws(uuid.toString()) + L" (expected " + s2ws(SERIALIZED_UUID.toString()) +
                            L" or a legacy UUID).";
                            
                            throw UnsupportedOperationException(reason);
                        }

                        bool supportsPrecedencePredicates = isFeatureSupported(ADDED_PRECEDENCE_TRANSITIONS, uuid);

                        ATNType grammarType = (ATNType)data[p++];
                        int maxTokenType = data[p++];
                        ATN *atn = new ATN(grammarType, maxTokenType);

                        //
                        // STATES
                        //
                        std::vector<std::pair<LoopEndState*, int>> loopBackStateNumbers;
                        std::vector<std::pair<BlockStartState*, int>> endStateNumbers;
                        int nstates = data[p++];
                        for (int i = 0; i < nstates; i++) {
                            int stype = data[p++];
                            // ignore bad type of states
                            if (stype == ATNState::ATN_INVALID_TYPE) {
                                atn->addState(nullptr);
                                continue;
                            }

                            int ruleIndex = data[p++];
                            if (ruleIndex == WCHAR_MAX) {
                                ruleIndex = -1;
                            }

                            ATNState *s = stateFactory(stype, ruleIndex);
                            if (stype == ATNState::LOOP_END) { // special case
                                int loopBackStateNumber = data[p++];
                              loopBackStateNumbers.push_back({ (LoopEndState*) s,  loopBackStateNumber });
                            } else if (s != nullptr) {
                              int endStateNumber = data[p++];
                              endStateNumbers.push_back({ (BlockStartState*) s, endStateNumber });
                            }
                            atn->addState(s);
                        }

                        // delay the assignment of loop back and end states until we know all the state instances have been initialized
                        for (auto &pair : loopBackStateNumbers) {
                            pair.first->loopBackState = atn->states.at(pair.second);
                        }

                        for (auto &pair : endStateNumbers) {
                            pair.first->endState = (BlockEndState*)atn->states.at(pair.second);
                        }

                        int numNonGreedyStates = data[p++];
                        for (int i = 0; i < numNonGreedyStates; i++) {
                            int stateNumber = data[p++];
                            ((DecisionState*)atn->states.at(stateNumber) /*static_cast<DecisionState*>(atn->states[stateNumber])*/)->nonGreedy = true;
                        }

                        if (supportsPrecedencePredicates) {
                            int numPrecedenceStates = data[p++];
                            for (int i = 0; i < numPrecedenceStates; i++) {
                                int stateNumber = data[p++];
                                (dynamic_cast<RuleStartState*>(atn->states[stateNumber]))->isPrecedenceRule = true;
                            }
                        }

                        //
                        // RULES
                        //
                        int nrules = data[p++];
                        if (atn->grammarType == ATNType::LEXER) {
                            atn->ruleToTokenType = new int[nrules];
                            atn->ruleToActionIndex = new int[nrules];
                        }

                        for (int i = 0; i < nrules; i++) {
                            int s = data[p++];
                            RuleStartState *startState = /*static_cast<RuleStartState*>*/(RuleStartState*)atn->states.at(s);
                            atn->ruleToStartState.push_back(startState);
                            if (atn->grammarType == ATNType::LEXER) {
                                int tokenType = data[p++];
                                if (tokenType == 0xFFFF) {
                                    tokenType = Token::_EOF;
                                }

                                atn->ruleToTokenType[i] = tokenType;
                                int actionIndex = data[p++];
                                if (actionIndex == 0xFFFF) {
                                    actionIndex = -1;
                                }

                                atn->ruleToActionIndex[i] = actionIndex;
                            }
                        }

                        atn->ruleToStopState = new RuleStopState*[nrules];
                        for (ATNState *state : atn->states) {
                            if (!(dynamic_cast<RuleStopState*>(state) != nullptr)) {
                                continue;
                            }

                            RuleStopState *stopState = static_cast<RuleStopState*>(state);
                            atn->ruleToStopState[state->ruleIndex] = stopState;
                            atn->ruleToStartState[state->ruleIndex]->stopState = stopState;
                        }

                        //
                        // MODES
                        //
                        int nmodes = data[p++];
                        for (int i = 0; i < nmodes; i++) {
                            // TODO: really?  atn->modeToStartState is const
                            //int s = data[p++];
                            // atn->modeToStartState.push_back(static_cast<TokensStartState*>(atn->states[s]));
                        }

                        //
                        // SETS
                        //
                        std::vector<misc::IntervalSet*> sets = std::vector<misc::IntervalSet*>();
                        int nsets = data[p++];
                        for (int i = 0; i < nsets; i++) {
                            int nintervals = data[p];
                            p++;
                            // TODO IntervalSet does not have a default constructor
                            //IntervalSet *set = new IntervalSet();
                            misc::IntervalSet *set = nullptr;
                            sets.push_back(set);

                            bool containsEof = data[p++] != 0;
                            if (containsEof) {
                                set->add(-1);
                            }

                            for (int j = 0; j < nintervals; j++) {
                                set->add(data[p], data[p + 1]);
                                p += 2;
                            }
                        }

                        //
                        // EDGES
                        //
                        int nedges = data[p++];
                        for (int i = 0; i < nedges; i++) {
                            int src = data[p];
                            int trg = data[p + 1];
                            int ttype = data[p + 2];
                            int arg1 = data[p + 3];
                            int arg2 = data[p + 4];
                            int arg3 = data[p + 5];
                            Transition *trans = edgeFactory(atn, ttype, src, trg, arg1, arg2, arg3, sets);
                                        //			System.out.println("EDGE "+trans.getClass().getSimpleName()+" "+
                                        //							   src+"->"+trg+
                                        //					   " "+Transition.serializationNames[ttype]+
                                        //					   " "+arg1+","+arg2+","+arg3);
                            ATNState *srcState = atn->states[src];
                            srcState->addTransition(trans);
                            p += 6;
                        }

                        // edges for rule stop states can be derived, so they aren't serialized
                        for (ATNState *state : atn->states) {
                            for (int i = 0; i < state->getNumberOfTransitions(); i++) {
                                Transition *t = state->transition(i);
                                if (!(dynamic_cast<RuleTransition*>(t) != nullptr)) {
                                    continue;
                                }

                                RuleTransition *ruleTransition = static_cast<RuleTransition*>(t);
                                atn->ruleToStopState[ruleTransition->target->ruleIndex]->addTransition(new EpsilonTransition(ruleTransition->followState));
                            }
                        }

                        for (ATNState *state : atn->states) {
                            if (dynamic_cast<BlockStartState*>(state) != nullptr) {
                                // we need to know the end state to set its start state
                                if ((static_cast<BlockStartState*>(state))->endState == nullptr) {
                                    throw new IllegalStateException();
                                }

                                // block end states can only be associated to a single block start state
                                if ((static_cast<BlockStartState*>(state))->endState->startState != nullptr) {
                                    throw new IllegalStateException();
                                    
                                }

                                (static_cast<BlockStartState*>(state))->endState->startState = static_cast<BlockStartState*>(state);
                            }

                            if (dynamic_cast<PlusLoopbackState*>(state) != nullptr) {
                                PlusLoopbackState *loopbackState = static_cast<PlusLoopbackState*>(state);
                                for (int i = 0; i < loopbackState->getNumberOfTransitions(); i++) {
                                    ATNState *target = loopbackState->transition(i)->target;
                                    if (dynamic_cast<PlusBlockStartState*>(target) != nullptr) {
                                        (static_cast<PlusBlockStartState*>(target))->loopBackState = loopbackState;
                                    }
                                }
                            } else if (dynamic_cast<StarLoopbackState*>(state) != nullptr) {
                                StarLoopbackState *loopbackState = static_cast<StarLoopbackState*>(state);
                                for (int i = 0; i < loopbackState->getNumberOfTransitions(); i++) {
                                    ATNState *target = loopbackState->transition(i)->target;
                                    if (dynamic_cast<StarLoopEntryState*>(target) != nullptr) {
                                        (static_cast<StarLoopEntryState*>(target))->loopBackState = loopbackState;
                                    }
                                }
                            }
                        }

                        //
                        // DECISIONS
                        //
                        int ndecisions = data[p++];
                        for (int i = 1; i <= ndecisions; i++) {
                            int s = data[p++];
                            DecisionState *decState = static_cast<DecisionState*>(atn->states[s]);
                            // TODO: decisionToState was originally declared as const in ATN
                            atn->decisionToState.push_back(decState);
                            decState->decision = i - 1;
                        }

                        if (deserializationOptions->isVerifyATN()) {
                            verifyATN(atn);
                        }

                        if (deserializationOptions->isGenerateRuleBypassTransitions() && atn->grammarType == ATNType::PARSER) {
                            atn->ruleToTokenType = new int[atn->ruleToStartState.size()];
                            for (std::vector<RuleStartState*>::size_type i = 0; i < atn->ruleToStartState.size(); i++) {
                                atn->ruleToTokenType[i] = atn->maxTokenType + (int)i + 1;
                            }

                            for (std::vector<RuleStartState*>::size_type i = 0; i < atn->ruleToStartState.size(); i++) {
                                BasicBlockStartState *bypassStart = new BasicBlockStartState();
                                bypassStart->ruleIndex = (int)i;
                                atn->addState(bypassStart);

                                BlockEndState *bypassStop = new BlockEndState();
                                bypassStop->ruleIndex = (int)i;
                                atn->addState(bypassStop);

                                bypassStart->endState = bypassStop;
                                atn->defineDecisionState(bypassStart);

                                bypassStop->startState = bypassStart;

                                ATNState *endState;
                                Transition *excludeTransition = nullptr;
                                if (atn->ruleToStartState[i]->isPrecedenceRule) {
                                    // wrap from the beginning of the rule to the StarLoopEntryState
                                    endState = nullptr;
                                    for (ATNState *state : atn->states) {
                                        if (state->ruleIndex != (int)i) {
                                            continue;
                                        }

                                        if (!(dynamic_cast<StarLoopEntryState*>(state) != nullptr)) {
                                            continue;
                                        }

                                        ATNState *maybeLoopEndState = state->transition(state->getNumberOfTransitions() - 1)->target;
                                        if (!(dynamic_cast<LoopEndState*>(maybeLoopEndState) != nullptr)) {
                                            continue;
                                        }

                                        if (maybeLoopEndState->epsilonOnlyTransitions && dynamic_cast<RuleStopState*>(maybeLoopEndState->transition(0)->target) != nullptr) {
                                            endState = state;
                                            break;
                                        }
                                    }

                                    if (endState == nullptr) {
                                        throw UnsupportedOperationException(L"Couldn't identify final state of the precedence rule prefix section.");

                                    }

                                    excludeTransition = (static_cast<StarLoopEntryState*>(endState))->loopBackState->transition(0);
                                } else {
                                    endState = atn->ruleToStopState[i];
                                }

                                // all non-excluded transitions that currently target end state need to target blockEnd instead
                                for (ATNState *state : atn->states) {
                                    for (Transition *transition : state->getTransitions()) {
                                        if (transition == excludeTransition) {
                                            continue;
                                        }

                                        if (transition->target == endState) {
                                            transition->target = bypassStop;
                                        }
                                    }
                                }

                                // all transitions leaving the rule start state need to leave blockStart instead
                                while (atn->ruleToStartState[i]->getNumberOfTransitions() > 0) {
                                    Transition *transition = atn->ruleToStartState[i]->removeTransition(atn->ruleToStartState[i]->getNumberOfTransitions() - 1);
                                    bypassStart->addTransition(transition);
                                }

                                // link the new states
                                atn->ruleToStartState[i]->addTransition(new EpsilonTransition(bypassStart));
                                bypassStop->addTransition(new EpsilonTransition(endState));

                                ATNState *matchState = new BasicState();
                                atn->addState(matchState);
                                matchState->addTransition(new AtomTransition(bypassStop, atn->ruleToTokenType[i]));
                                bypassStart->addTransition(new EpsilonTransition(matchState));
                            }

                            if (deserializationOptions->isVerifyATN()) {
                                // reverify after modification
                                verifyATN(atn);
                            }
                        }

                        return atn;
                    }

                    void ATNDeserializer::verifyATN(ATN *atn) {
                        // verify assumptions
                        for (ATNState *state : atn->states) {
                            if (state == nullptr) {
                                continue;
                            }

                            checkCondition(state->onlyHasEpsilonTransitions() || state->getNumberOfTransitions() <= 1);

                            if (dynamic_cast<PlusBlockStartState*>(state) != nullptr) {
                                checkCondition((static_cast<PlusBlockStartState*>(state))->loopBackState != nullptr);
                            }

                            if (dynamic_cast<StarLoopEntryState*>(state) != nullptr) {
                                StarLoopEntryState *starLoopEntryState = static_cast<StarLoopEntryState*>(state);
                                checkCondition(starLoopEntryState->loopBackState != nullptr);
                                checkCondition(starLoopEntryState->getNumberOfTransitions() == 2);

                                if (dynamic_cast<StarBlockStartState*>(starLoopEntryState->transition(0)->target) != nullptr) {
                                    checkCondition(dynamic_cast<LoopEndState*>(starLoopEntryState->transition(1)->target) != nullptr);
                                    checkCondition(!starLoopEntryState->nonGreedy);
                                } else if (dynamic_cast<LoopEndState*>(starLoopEntryState->transition(0)->target) != nullptr) {
                                    checkCondition(dynamic_cast<StarBlockStartState*>(starLoopEntryState->transition(1)->target) != nullptr);
                                    checkCondition(starLoopEntryState->nonGreedy);
                                } else {
                                    throw new IllegalStateException();

                                }
                            }

                            if (dynamic_cast<StarLoopbackState*>(state) != nullptr) {
                                checkCondition(state->getNumberOfTransitions() == 1);
                                checkCondition(dynamic_cast<StarLoopEntryState*>(state->transition(0)->target) != nullptr);
                            }

                            if (dynamic_cast<LoopEndState*>(state) != nullptr) {
                                checkCondition((static_cast<LoopEndState*>(state))->loopBackState != nullptr);
                            }

                            if (dynamic_cast<RuleStartState*>(state) != nullptr) {
                                checkCondition((static_cast<RuleStartState*>(state))->stopState != nullptr);
                            }

                            if (dynamic_cast<BlockStartState*>(state) != nullptr) {
                                checkCondition((static_cast<BlockStartState*>(state))->endState != nullptr);
                            }

                            if (dynamic_cast<BlockEndState*>(state) != nullptr) {
                                checkCondition((static_cast<BlockEndState*>(state))->startState != nullptr);
                            }

                            if (dynamic_cast<DecisionState*>(state) != nullptr) {
                                DecisionState *decisionState = static_cast<DecisionState*>(state);
                                checkCondition(decisionState->getNumberOfTransitions() <= 1 || decisionState->decision >= 0);
                            } else {
                                checkCondition(state->getNumberOfTransitions() <= 1 || dynamic_cast<RuleStopState*>(state) != nullptr);
                            }
                        }
                    }

                    void ATNDeserializer::checkCondition(bool condition) {
                        checkCondition(condition, L"");
                    }

                    void ATNDeserializer::checkCondition(bool condition, const std::wstring &message) {
                        if (!condition) {
                            // TODO: throw IllegalStateException(message);
                            throw std::exception();
                        }
                    }

                    Guid ATNDeserializer::toUUID(const wchar_t *data, int offset) {
                        return Guid((uint32_t *)data + offset, true);
                    }

                    Transition *ATNDeserializer::edgeFactory(ATN *atn, int type, int src, int trg, int arg1, int arg2, int arg3, std::vector<misc::IntervalSet*> &sets) {
                        ATNState *target = atn->states[trg];
                        switch (type) {
                            case Transition::EPSILON :
                                return new EpsilonTransition(target);
                            case Transition::RANGE :
                                if (arg3 != 0) {
                                    return new RangeTransition(target, Token::_EOF, arg2);
                                } else {
                                    return new RangeTransition(target, arg1, arg2);
                                }
                            case Transition::RULE :
                                return new RuleTransition(static_cast<RuleStartState*>(atn->states[arg1]), arg2, arg3, target);
                            case Transition::PREDICATE :
                                return new PredicateTransition(target, arg1, arg2, arg3 != 0);
                            case Transition::PRECEDENCE:
                                return new PrecedencePredicateTransition(target, arg1);
                            case Transition::ATOM :
                                if (arg3 != 0) {
                                    return new AtomTransition(target, Token::_EOF);
                                } else {
                                    return new AtomTransition(target, arg1);
                                }
                            case Transition::ACTION :
                                return new ActionTransition(target, arg1, arg2, arg3 != 0);
                            case Transition::SET :
                                return new SetTransition(target, sets[arg1]);
                            case Transition::NOT_SET :
                                return new NotSetTransition(target, sets[arg1]);
                            case Transition::WILDCARD :
                                return new WildcardTransition(target);
                        }

                        throw IllegalArgumentException(L"The specified transition type is not valid.");
                    }

                    ATNState *ATNDeserializer::stateFactory(int type, int ruleIndex) {
                        ATNState *s;
                        switch (type) {
                            case ATNState::ATN_INVALID_TYPE:
                                return nullptr;
                            case ATNState::BASIC :
                                s = new BasicState();
                                break;
                            case ATNState::RULE_START :
                                s = new RuleStartState();
                                break;
                            case ATNState::BLOCK_START :
                                s = new BasicBlockStartState();
                                break;
                            case ATNState::PLUS_BLOCK_START :
                                s = new PlusBlockStartState();
                                break;
                            case ATNState::STAR_BLOCK_START :
                                s = new StarBlockStartState();
                                break;
                            case ATNState::TOKEN_START :
                                s = new TokensStartState();
                                break;
                            case ATNState::RULE_STOP :
                                s = new RuleStopState();
                                break;
                            case ATNState::BLOCK_END :
                                s = new BlockEndState();
                                break;
                            case ATNState::STAR_LOOP_BACK :
                                s = new StarLoopbackState();
                                break;
                            case ATNState::STAR_LOOP_ENTRY :
                                s = new StarLoopEntryState();
                                break;
                            case ATNState::PLUS_LOOP_BACK :
                                s = new PlusLoopbackState();
                                break;
                            case ATNState::LOOP_END :
                                s = new LoopEndState();
                                break;
                            default :
                                std::wstring message = L"The specified state type " +
                                                       std::to_wstring(type) + L" is not valid.";
                                throw IllegalArgumentException(message);
                        }

                        s->ruleIndex = ruleIndex;
                        return s;
                    }

                    std::vector<Guid> ATNDeserializer::supportedUUIDsInitializer() {
                        std::vector<Guid> supportedUUIDs;
                        supportedUUIDs.push_back(BASE_SERIALIZED_UUID);
                        supportedUUIDs.push_back(ADDED_PRECEDENCE_TRANSITIONS);
                      supportedUUIDs.push_back(ADDED_LEXER_ACTIONS);

                        return supportedUUIDs;
                    }

                    ATNDeserializationOptions *ATNDeserializer::deserializationOptionsInitializer(ATNDeserializationOptions *dso) {
                        if (dso == nullptr)
                            return ATNDeserializationOptions::getDefaultOptions();
                        return dso;
                    }
                }
            }
        }
    }
}
