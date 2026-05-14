#include <stdexcept>

using namespace std;

#include "recog_token_buffer.h"

#ifdef CPPGM_DEBUG_RECOG_BUFFER
#include <cstdlib>
#include <fstream>
#endif

namespace {

const size_t RECOG_TOKEN_FILL_BATCH_SIZE = 8192;

string token_text_for_span(const RecogToken & token)
{
  if(token.is_rshift_piece()) {
    return ">";
  }
  return token.source;
}

}  // namespace

RecogTokenBuffer::RecogTokenBuffer(IRecogTokenSource & source,
                                   const string & debug_label,
                                   const SourceLocationTable * locations) :
  source(source),
  finished(false),
  locations(locations),
  primary_source_file_(debug_label)
#ifdef CPPGM_DEBUG_RECOG_BUFFER
  ,
  debug_label(debug_label),
  debug_max_materialized_tokens(0),
  debug_max_peek_offset(0),
  debug_max_buffered_ahead(0),
  debug_forced_full_materializations(0),
  debug_max_forced_materialized_tokens(0),
  debug_used(false)
#endif
{
#ifndef CPPGM_DEBUG_RECOG_BUFFER
  (void)debug_label;
#endif
}

RecogTokenBuffer::~RecogTokenBuffer()
{
#ifdef CPPGM_DEBUG_RECOG_BUFFER
  const char * path = getenv("CPPGM_RECOG_BUFFER_STATS_FILE");
  if(!path || !*path || !debug_used) {
    return;
  }

  ofstream out(path, ios::app);
  out << "label=" << (debug_label.empty() ? "<unknown>" : debug_label)
      << " max_materialized_tokens=" << debug_max_materialized_tokens
      << " max_peek_offset=" << debug_max_peek_offset
      << " max_buffered_ahead=" << debug_max_buffered_ahead
      << " forced_full_materializations=" << debug_forced_full_materializations
      << " max_forced_materialized_tokens=" << debug_max_forced_materialized_tokens
      << '\n';
#endif
}

const RecogToken & RecogTokenBuffer::operator[](size_t index) const
{
  ensure(index);
  if(index >= tokens.size()) {
    throw logic_error("recog token index out of range");
  }
  return tokens[index];
}

const RecogToken & RecogTokenBuffer::peek(size_t index) const
{
  ensure(index);
  if(index < tokens.size()) {
    return tokens[index];
  }
  return last_pulled();
}

size_t RecogTokenBuffer::size() const
{
#ifdef CPPGM_DEBUG_RECOG_BUFFER
  bool was_finished = finished;
#endif
  materialize_all();
#ifdef CPPGM_DEBUG_RECOG_BUFFER
  if(!was_finished) {
    debug_used = true;
    ++debug_forced_full_materializations;
    if(tokens.size() > debug_max_forced_materialized_tokens) {
      debug_max_forced_materialized_tokens = tokens.size();
    }
  }
#endif
  return tokens.size();
}

const RecogToken & RecogTokenBuffer::back() const
{
#ifdef CPPGM_DEBUG_RECOG_BUFFER
  bool was_finished = finished;
#endif
  materialize_all();
#ifdef CPPGM_DEBUG_RECOG_BUFFER
  if(!was_finished) {
    debug_used = true;
    ++debug_forced_full_materializations;
    if(tokens.size() > debug_max_forced_materialized_tokens) {
      debug_max_forced_materialized_tokens = tokens.size();
    }
  }
#endif
  return tokens.back();
}

vector<RecogToken> RecogTokenBuffer::slice(size_t start, size_t end) const
{
  if(end < start) {
    throw logic_error("invalid token slice");
  }

  if(end != 0) {
    ensure(end - 1);
  }
  return vector<RecogToken>(tokens.begin() + start, tokens.begin() + end);
}

string RecogTokenBuffer::span_text(size_t start, size_t end) const
{
  if(end < start) {
    throw logic_error("invalid token span");
  }

  if(end != 0) {
    ensure(end - 1);
  }

  string text;
  text.reserve((end - start) * 8);
  for(size_t i = start; i < end; ++i) {
    text += token_text_for_span(tokens[i]);
  }
  return text;
}

const SourceLocationTable * RecogTokenBuffer::source_locations() const
{
  return locations;
}

const string & RecogTokenBuffer::primary_source_file() const
{
  return primary_source_file_;
}

#ifdef CPPGM_DEBUG_RECOG_BUFFER
void RecogTokenBuffer::debug_note_peek(size_t pos, size_t offset) const
{
  debug_used = true;
  if(offset > debug_max_peek_offset) {
    debug_max_peek_offset = offset;
  }

  size_t buffered_ahead = 0;
  if(tokens.size() > pos + 1) {
    buffered_ahead = tokens.size() - pos - 1;
  }
  if(buffered_ahead > debug_max_buffered_ahead) {
    debug_max_buffered_ahead = buffered_ahead;
  }
}
#endif

void RecogTokenBuffer::ensure(size_t index) const
{
  while(!finished && tokens.size() <= index) {
    const size_t needed = index + 1 - tokens.size();
    const size_t batch_size =
        needed > RECOG_TOKEN_FILL_BATCH_SIZE ? needed : RECOG_TOKEN_FILL_BATCH_SIZE;
    vector<RecogToken> batch;
    batch.reserve(batch_size);
    source.get_many(batch, batch_size);
    append_token_batch(batch);
    if(batch.empty()) {
      throw logic_error("recog token source made no progress");
    }
  }

  if(tokens.empty()) {
    throw logic_error("empty recog token buffer");
  }
}

void RecogTokenBuffer::append_token_batch(vector<RecogToken> & batch) const
{
  for(size_t i = 0; i < batch.size(); ++i) {
    const bool invalid = batch[i].is_invalid();
    const bool final = batch[i].is_eof() || invalid;
    tokens.push_back(std::move(batch[i]));
#ifdef CPPGM_DEBUG_RECOG_BUFFER
    debug_used = true;
    if(tokens.size() > debug_max_materialized_tokens) {
      debug_max_materialized_tokens = tokens.size();
    }
#endif
    if(final) {
      if(invalid) {
        tokens.push_back(
            RecogToken{RT_EOF, string(), static_cast<ETokenType>(0), 0});
      }
      finished = true;
      return;
    }
  }
}

void RecogTokenBuffer::materialize_all() const
{
  while(!finished) {
    vector<RecogToken> batch;
    batch.reserve(RECOG_TOKEN_FILL_BATCH_SIZE);
    source.get_many(batch, RECOG_TOKEN_FILL_BATCH_SIZE);
    append_token_batch(batch);
    if(batch.empty()) {
      throw logic_error("recog token source made no progress");
    }
  }

  if(tokens.empty()) {
    throw logic_error("empty recog token buffer");
  }
}

const RecogToken & RecogTokenBuffer::last_pulled() const
{
  if(tokens.empty()) {
    throw logic_error("empty recog token buffer");
  }
  return tokens.back();
}
